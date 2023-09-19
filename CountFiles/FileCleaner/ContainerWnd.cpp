
// ContainerWnd.cpp : implementation of the CContainerWnd class
//

#include "pch.h"
#include "framework.h"
#include "ContainerWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CContainerWnd

IMPLEMENT_DYNAMIC(CContainerWnd,CWnd)

CContainerWnd::CContainerWnd():m_pWndView(NULL),m_pHeadBar(NULL)
{
}

CContainerWnd::~CContainerWnd()
{
}

BEGIN_MESSAGE_MAP(CContainerWnd,CWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int CContainerWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create a bar to occupy top of the client area of the frame
	m_pHeadBar=new CHeadBar;

	if (!m_pHeadBar->Create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST+1, nullptr))
	{
		TRACE0("Failed to create head bar\n");
		return -1;
	}

	// create a view to occupy the client area of the frame
	m_pWndView=new CMainView;
	m_pWndView->SetScrollSizes(MM_TEXT,CSize(LINE_HEIGHT,LINE_HEIGHT),
		CSize(LINE_HEIGHT,LINE_HEIGHT*3),CSize(LINE_HEIGHT,LINE_HEIGHT));

	if (!m_pWndView->Create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST+2, nullptr))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	Relayout();

	return 0;
}

void CContainerWnd::Relayout()
{
	CRect rect,rcBar,rcView;
	GetClientRect(&rect);
	rcBar=rect;
	rcBar.bottom=LINE_HEIGHT;
	rcView=rect;
	rcView.top=LINE_HEIGHT;
	rcView.bottom=max(rcView.bottom,LINE_HEIGHT);
	m_pHeadBar->MoveWindow(&rcBar);
	m_pWndView->MoveWindow(&rcView);
}

// CContainerWnd message handlers

BOOL CContainerWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(0);

	return TRUE;
}
// CContainerWnd message handlers

void CContainerWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	m_pWndView->SetFocus();
}

BOOL CContainerWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_pWndView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// let the bar have second crack at the command
	if (m_pHeadBar->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CContainerWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	Relayout();
}
