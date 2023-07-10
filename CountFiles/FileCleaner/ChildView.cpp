
// ChildView.cpp : implementation of the CChildView class
//

#include "pch.h"
#include "framework.h"
#include "FileCleaner.h"
#include "ChildView.h"
#include "DrawObject.h"

#include "GenFileList.h"
#include "FileListLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CScrollTreeList
CScrollTreeList::CScrollTreeList(CWnd* pWnd):TreeListCtrl(pWnd)
{
}
CPoint CScrollTreeList::GetScrollPos() const
{
	return ((CScrollView*)m_pWnd)->GetScrollPosition();
}
void CScrollTreeList::SetScrollSizes(const CSize& size)
{
	((CScrollView*)m_pWnd)->SetScrollSizes(MM_TEXT,size,
		CSize(LINE_HEIGHT,LINE_HEIGHT*3),CSize(LINE_HEIGHT,LINE_HEIGHT));
}

// CChildView

CChildView::CChildView():m_TreeList(this)
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CScrollView)
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
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CChildView, CScrollView)

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
/*
static int test()
{
	struct fnode
	{
		dword flags;
		UInteger64 fl_start;
		UInteger64 fl_end;
		void* handle;
		fnode():flags(0),handle(NULL){}
	};
	struct dir_contents;
	struct err_dir_node;
	struct dir_node:public fnode
	{
		dir_contents* contents;
		err_dir_node* enode;
		dir_node():contents(NULL),enode(NULL){}
	};
	const char* listfile="E:\\Programs\\CountFiles\\CountFiles\\IPCIF.txt";
	FileListLoader loader;
	int ret=loader.Load(listfile);
	if(ret!=0)
		return ret;
	ret=loader.ExpandNode(loader.GetRootNode(),true);
	if(ret!=0)
		return ret;

	file_node_info info;

	for(int i=0;i<(int)get_subdir_cnt(loader.GetRootNode());i++)
		ret=loader.GetNodeInfo(get_subdir(loader.GetRootNode(),i),&info);
	for(int i=0;i<(int)get_subfile_cnt(loader.GetRootNode());i++)
		ret=loader.GetNodeInfo(get_subfile(loader.GetRootNode(),i),&info);

	for(int i=0;i<(int)get_subdir_cnt(loader.GetRootNode());i++)
		ret=loader.GetNodeInfo(get_subdir(loader.GetRootNode(),i),&info);
	for(int i=0;i<(int)get_subfile_cnt(loader.GetRootNode());i++)
		ret=loader.GetNodeInfo(get_subfile(loader.GetRootNode(),i),&info);

	for(int i=0;i<(int)get_subdir_cnt(loader.GetRootNode());i++)
		ret=loader.GetNodeInfo(get_subdir(loader.GetRootNode(),i),&info);
	ret=loader.GetNodeInfo(get_subfile(loader.GetRootNode(),0),&info);

	dir_node tmpnode=*(dir_node*)get_subdir(loader.GetRootNode(),0);
	ret=loader.GetNodeInfo(get_subdir(loader.GetRootNode(),0),&info);
	ret=loader.GetNodeInfo((HDNODE)&tmpnode,&info);

	return ret;
}*/
int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	//test();
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
	SetCapture();
	CPoint pt=GetMousePos(point,this);
	if(pt.x>=0&&pt.y>=0)
		m_TreeList.OnLBDown(pt,nFlags);
	CScrollView::OnLButtonDown(nFlags, point);
}


void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture();
	CPoint pt=GetMousePos(point,this);
	ASSERT(pt.x>=0&&pt.y>=0);
	if(pt.x>=0&&pt.y>=0)
		m_TreeList.OnLBUp(pt,nFlags);
	CScrollView::OnLButtonUp(nFlags, point);
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=GetMousePos(point,this);
	if(pt.x>=0&&pt.y>=0)
		m_TreeList.OnMMove(pt,nFlags);
	CScrollView::OnMouseMove(nFlags, point);
}


void CChildView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=GetMousePos(point,this);
	if(pt.x>=0&&pt.y>=0)
		m_TreeList.OnLBDblClick(pt,nFlags);
	CScrollView::OnLButtonDblClk(nFlags, point);
}


void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=GetMousePos(point,this);
	if(pt.x>=0&&pt.y>=0)
		m_TreeList.OnRBDown(pt,nFlags);
	CScrollView::OnRButtonDown(nFlags, point);
}


void CChildView::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=GetMousePos(point,this);
	ASSERT(pt.x>=0&&pt.y>=0);
	if(pt.x>=0&&pt.y>=0)
		m_TreeList.OnRBUp(pt,nFlags);
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
