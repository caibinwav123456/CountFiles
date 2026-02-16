
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "pch.h"
#include "FileCleaner.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	ON_UPDATE_COMMAND_UI_RANGE(ID_TOOLBAR_VIEW_COMP, ID_TOOLBAR_VIEW_OP, &CMainFrame::OnUpdateControlBarMenu)
	ON_COMMAND_EX_RANGE(ID_TOOLBAR_VIEW_COMP, ID_TOOLBAR_VIEW_OP, &CMainFrame::OnBarCheck)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame() noexcept:m_pWndView(NULL),m_pWndProp(NULL),m_wndBaseBar(this)
{
	// TODO: add member initialization code here
	m_hIcon=0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create a view to occupy the client area of the frame
	m_pWndView=new CContainerWnd;

	m_hIcon=AfxGetApp()->LoadIcon(IDI_MAINFRAME);
	SetIcon(m_hIcon,TRUE);
	SetIcon(m_hIcon,FALSE);

	if (!m_pWndView->Create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, nullptr))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	m_pWndProp=new CPropWnd;
	if (!m_pWndProp->Create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST+1, nullptr))
	{
		TRACE0("Failed to create property window\n");
		return -1;
	}

	struct ToolBarLoadData
	{
		CMyToolBar* pToolBar;
		UINT nIDRsrc;
		UINT nIDConf;
		UINT nIDBmp;
		UINT nIDBmpBack;
	};
	ToolBarLoadData arrToolBar[]={
		{&m_wndToolBarComp,ID_TOOLBAR_VIEW_COMP,IDS_TOOLBAR_CONF_COMP,
		IDB_TOOLBAR_VIEW_COMP,IDB_TOOLBAR_BACK_COMP},
		{&m_wndToolBarView,ID_TOOLBAR_VIEW_VIEW,IDS_TOOLBAR_CONF_VIEW,
		IDB_TOOLBAR_VIEW_VIEW,IDB_TOOLBAR_BACK_VIEW},
		{&m_wndToolBarOper,ID_TOOLBAR_VIEW_OP,IDS_TOOLBAR_CONF_OP,
		IDB_TOOLBAR_VIEW_OP,IDB_TOOLBAR_BACK_OP},
	};

	for(int i=0;i<sizeof(arrToolBar)/sizeof(ToolBarLoadData);i++)
	{
		ToolBarLoadData& bar=arrToolBar[i];
		if (!bar.pToolBar->MyCreate(bar.nIDRsrc, bar.nIDConf, bar.nIDBmp, bar.nIDBmpBack, this))
		{
			TRACE("Failed to create toolbar ID 0x%04X\n", bar.nIDRsrc);
			return -1;      // fail to create
		}
	}

	if (!m_wndBaseBar.CreateBar(this))
	{
		TRACE0("Failed to create base bar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));


	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_pWndView->SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_pWndView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	if (m_pWndProp->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if(m_pWndProp->GetPropCount()<=1)
	{
		CFrameWnd::OnClose();
		return;
	}
	LPCTSTR msg=_T(
		"You have opened more than one tab, do you want to close the whole program or just the current tab?\n"
		"Yes - close the whole program\n"
		"No - close the current tab"
	);
	int ret=MessageBox(msg,NULL,MB_YESNOCANCEL);
	switch(ret)
	{
	case IDYES:
		CFrameWnd::OnClose();
		break;
	case IDNO:
		SendMessageToIDWnd(IDW_PROP_WND,WM_CLOSE_CURRENT_PROP_TAB);
		break;
	default:
		break;
	}
}

void CMainFrame::OnUpdateControlBarMenu(CCmdUI* pCmdUI)
{
	CFrameWnd::OnUpdateControlBarMenu(pCmdUI);
}

BOOL CMainFrame::OnBarCheck(UINT nID)
{
	return CFrameWnd::OnBarCheck(nID);
}