
// ChildView.cpp : implementation of the CChildView class
//

#include "pch.h"
#include "framework.h"
#include "FileCleaner.h"
#include "ChildView.h"
#include "DrawObject.h"
#include "DlgLoad.h"

#include "GenFileList.h"
#include "FileListLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CScrollTreeList
CScrollTreeList::CScrollTreeList(CWnd* pWnd):TreeListCtrl(pWnd),m_sizeScl(MIN_SCROLL_WIDTH,1)
{
}
CPoint CScrollTreeList::GetScrollPos() const
{
	return ((CScrollView*)m_pWnd)->GetScrollPosition();
}
void CScrollTreeList::SetScrollSizes(const CSize& size)
{
	size.cx>0?(m_sizeScl.cx=max(size.cx,MIN_SCROLL_WIDTH))
		:(size.cx==0?(m_sizeScl.cx=MIN_SCROLL_WIDTH):0);
	size.cy>0?(m_sizeScl.cy=size.cy):(size.cy==0?m_sizeScl.cy=1:0);
	((CScrollView*)m_pWnd)->SetScrollSizes(MM_TEXT,m_sizeScl,
		CSize(LINE_HEIGHT,LINE_HEIGHT*3),CSize(LINE_HEIGHT,LINE_HEIGHT));
}
CSize CScrollTreeList::GetScrollSizes()
{
	return m_sizeScl;
}

// CChildView

CChildView::CChildView():m_TreeList(this)
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CScrollView)
	ON_MESSAGE(WM_FILE_LIST_START_LOAD,OnStartLoadList)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CChildView, CScrollView)
IMPLEMENT_ID2WND_MAP(CChildView, IDW_MAIN_VIEW)

// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}

void CChildView::OnDraw(CDC* pDC)
{
	// TODO: Add your specialized code here and/or call the base class
	m_TreeList.Draw(pDC,true);
}
int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	if(m_TreeList.Init()!=0)
		return -1;

	return 0;
}


void CChildView::OnDestroy()
{
	CScrollView::OnDestroy();

	// TODO: Add your message handler code here
	m_TreeList.Exit();
}

LRESULT CChildView::OnStartLoadList(WPARAM wParam,LPARAM lParam)
{
	FListLoadData* lpData=(FListLoadData*)wParam;
	if(lpData->mask&FILE_LIST_ATTRIB_MAIN)
	{
		dword type=0;
		if(sys_fstat((char*)t2a(lpData->left).c_str(),&type)!=0)
			return FALSE;

		if(type==FILE_TYPE_DIR)
		{
			CDlgLoad dlg(NULL,lpData->left);
			if(dlg.DoModal()==IDOK)
			{
				CString strList=a2t(dlg.GetCacheFilePath());
				CString strErrList=a2t(dlg.GetCacheErrFilePath());
			}
		}
	}
	return TRUE;
}

static inline UINT GetKey()
{
#define key_state(vk) (GetAsyncKeyState(vk)&0x8000)
	UINT nFlags=0;
	if(key_state(VK_LBUTTON))
		nFlags|=MK_LBUTTON;
	if(key_state(VK_RBUTTON))
		nFlags|=MK_RBUTTON;
	if(key_state(VK_CONTROL))
		nFlags|=MK_CONTROL;
	if(key_state(VK_SHIFT))
		nFlags|=MK_SHIFT;
	return nFlags;
}
static inline CPoint GetMousePos(CScrollView* pView)
{
	CPoint pt;
	GetCursorPos(&pt);
	pView->ScreenToClient(&pt);
	TRACE("mpos: %d, %d\n",pt.x,pt.y);
	pt.Offset(CSize(pView->GetScrollPosition()));
	return pt;
}
static inline CPoint GetMousePos(const CPoint& pt,CScrollView* pView)
{
	CPoint point=pt;
	point.Offset(CSize(pView->GetScrollPosition()));
	return point;
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=point;
	pt.Offset(CSize(GetScrollPosition()));
	m_TreeList.OnLBDown(pt);
	CScrollView::OnLButtonDown(nFlags, point);
}


void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=point;
	pt.Offset(CSize(GetScrollPosition()));
	m_TreeList.OnLBUp(pt);
	CScrollView::OnLButtonUp(nFlags, point);
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=point;
	pt.Offset(CSize(GetScrollPosition()));
	m_TreeList.OnMMove(pt);
	CScrollView::OnMouseMove(nFlags, point);
}


void CChildView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=point;
	pt.Offset(CSize(GetScrollPosition()));
	m_TreeList.OnLBDblClick(pt);
	CScrollView::OnLButtonDblClk(nFlags, point);
}


void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=point;
	pt.Offset(CSize(GetScrollPosition()));
	m_TreeList.OnRBDown(pt);
	CScrollView::OnRButtonDown(nFlags, point);
}


void CChildView::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=point;
	pt.Offset(CSize(GetScrollPosition()));
	m_TreeList.OnRBUp(pt);
	CScrollView::OnRButtonUp(nFlags, point);
}


void CChildView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
	CPoint point=GetMousePos(this);
	if(point.x>=0&&point.y>=0)
		m_TreeList.OnMMove(point,GetKey());
}


BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	BOOL ret=CScrollView::OnMouseWheel(nFlags, zDelta, pt);
	CPoint point=GetMousePos(this);
	if(point.x>=0&&point.y>=0)
		m_TreeList.OnMMove(point,GetKey());
	return ret;
}


BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;//CScrollView::OnEraseBkgnd(pDC);
}


void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);
	m_TreeList.SetScrollSizes(CSize(cx,-1));
	// TODO: Add your message handler code here
}
