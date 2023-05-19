
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
	((CScrollView*)m_pWnd)->SetScrollSizes(MM_TEXT,size);
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

	dir_node tmpnode=*get_subdir(loader.GetRootNode(),0);
	ret=loader.GetNodeInfo(get_subdir(loader.GetRootNode(),0),&info);
	ret=loader.GetNodeInfo(&tmpnode,&info);

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
