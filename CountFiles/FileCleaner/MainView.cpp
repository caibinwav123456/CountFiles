
// MainView.cpp : implementation of the CMainView class
//

#include "pch.h"
#include "framework.h"
#include "FileCleaner.h"
#include "MainView.h"
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
	m_sizeScl.cx=MIN_SCROLL_WIDTH;
	size.cy>0?(m_sizeScl.cy=size.cy):(size.cy==0?m_sizeScl.cy=1:0);
	((CScrollView*)m_pWnd)->SetScrollSizes(MM_TEXT,m_sizeScl,
		CSize(LINE_HEIGHT,LINE_HEIGHT*3),CSize(LINE_HEIGHT,LINE_HEIGHT));
}
CSize CScrollTreeList::GetScrollSizes()
{
	return m_sizeScl;
}

// CMainView

CMainView::CMainView():m_TreeList(this)
{
}

CMainView::~CMainView()
{
}


BEGIN_MESSAGE_MAP(CMainView, CScrollView)
	ON_MESSAGE(WM_FILE_LIST_START_LOAD,OnStartLoadList)
	ON_MESSAGE(WM_REARRANGE_TAB_SIZE,OnRearrangeTabSize)
	ON_MESSAGE(WM_EXPORT_LIST_FILE,OnExportListFile)
	ON_MESSAGE(WM_LIST_FILE_VALID,OnExportIsValid)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(CMainView, CScrollView)
IMPLEMENT_ID2WND_MAP(CMainView, IDW_MAIN_VIEW)

// CMainView message handlers

BOOL CMainView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}

void CMainView::OnDraw(CDC* pDC)
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
int CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	//test();
	if(m_TreeList.Init()!=0)
		return -1;

	return 0;
}


void CMainView::OnDestroy()
{
	CScrollView::OnDestroy();

	// TODO: Add your message handler code here
	m_TreeList.Exit();
}

LRESULT CMainView::OnStartLoadList(WPARAM wParam,LPARAM lParam)
{
	int ret=0;
	FListLoadData* lpData=(FListLoadData*)wParam;
	string strList,strListRef;
	string strErrList,strErrListRef;
	dword type=0;
	UINT mask=0;
	m_TreeList.UnLoad();
	if(!lpData->left.IsEmpty())
	{
		string path=t2astr(lpData->left);
		if((ret=sys_fstat((char*)path.c_str(),&type))!=0)
		{
			PDXShowMessage(_T("\'%s\': %s"),(LPCTSTR)lpData->left,a2t(get_error_desc(ret)));
			goto fail;
		}
		if(type==FILE_TYPE_DIR)
		{
			if(lpData->mask&FILE_LIST_ATTRIB_MAIN)
			{
				CDlgLoad dlg(NULL,lpData->left);
				if(dlg.DoModal()!=IDOK)
					goto fail;
				strList=CProgramData::GetCacheFilePath();
			}
			else
			{
				path=CProgramData::GetCacheFilePath();
				if(sys_fstat((char*)path.c_str(),&type)!=0||type!=FILE_TYPE_NORMAL)
					goto fail;
				strList=path;
			}
			string patherr=CProgramData::GetErrListFilePath(strList);
			if(sys_fstat((char*)patherr.c_str(),&type)==0&&type==FILE_TYPE_NORMAL)
				strErrList=patherr;
		}
		else
		{
			strList=path;
			string patherr=CProgramData::GetErrListFilePath(path);
			if(sys_fstat((char*)patherr.c_str(),&type)==0&&type==FILE_TYPE_NORMAL)
				strErrList=patherr;
		}
		mask|=FILE_LIST_ATTRIB_MAIN;
	}
	if(!lpData->right.IsEmpty())
	{
		string path=t2astr(lpData->right);
		if((ret=sys_fstat((char*)path.c_str(),&type))!=0)
		{
			PDXShowMessage(_T("\'%s\': %s"),(LPCTSTR)lpData->left,a2t(get_error_desc(ret)));
			goto fail;
		}
		else if(type==FILE_TYPE_DIR)
		{
			PDXShowMessage(_T("\'%s\': %s"),(LPCTSTR)lpData->left,_T("reference path can not be a directory"));
			goto fail;
		}
		else
		{
			strListRef=path;
			string patherr=CProgramData::GetErrListFilePath(path);
			if(sys_fstat((char*)patherr.c_str(),&type)==0&&type==FILE_TYPE_NORMAL)
				strErrListRef=patherr;
		}
		mask|=FILE_LIST_ATTRIB_REF;
	}
	if(mask!=0)
	{
		fail_op(ret,0,m_TreeList.Load(mask,strList.c_str(),strErrList.c_str(),strListRef.c_str(),strErrListRef.c_str()),
		{
			PDXShowMessage(_T("Load file list failed: %s"),a2t(get_error_desc(ret)));
			goto fail;
		})
	}
	Invalidate();
	return TRUE;
fail:
	Invalidate();
	return FALSE;
}
LRESULT CMainView::OnRearrangeTabSize(WPARAM wParam, LPARAM lParam)
{
	TabInfo* tab=(TabInfo*)wParam;
	m_TreeList.SetTabInfo(tab);
	Invalidate();
	return 0;
}
LRESULT CMainView::OnExportIsValid(WPARAM wParam, LPARAM lParam)
{
	string lfile,efile;
	m_TreeList.GetListFilePath((INT_PTR)wParam,lfile,efile);
	if(lfile.empty())
		PDXShowMessage(_T("No files will be exported!"));
	return (LRESULT)(!lfile.empty());
}
LRESULT CMainView::OnExportListFile(WPARAM wParam, LPARAM lParam)
{
	int ret=0;
	FListExportData* pData=(FListExportData*)wParam;
	string lfile,efile;
	m_TreeList.GetListFilePath(pData->side,lfile,efile);
	if(lfile.empty())
	{
		PDXShowMessage(_T("No files will be exported!"));
		return 0;
	}
	fail_goto(ret,0,sys_fcopy((char*)lfile.c_str(),(char*)pData->lfile.c_str()),fail);
	if(!efile.empty())
		fail_goto(ret,0,sys_fcopy((char*)efile.c_str(),(char*)pData->efile.c_str()),fail);
	MessageBox(_T("Export successful!"));
	return 0;
fail:
	sys_fdelete((char*)pData->lfile.c_str());
	sys_fdelete((char*)pData->efile.c_str());
	PDXShowMessage(_T("Export faild: %s"),a2t(get_error_desc(ret)));
	return ret;
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
	if(point.x<0)point.x=0;
	if(point.y<0)point.y=0;
	return point;
}

void CMainView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetCapture();
	CPoint pt=GetMousePos(point,this);
	m_TreeList.OnLBDown(pt,nFlags);
	CScrollView::OnLButtonDown(nFlags, point);
}


void CMainView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture();
	CPoint pt=GetMousePos(point,this);
	m_TreeList.OnLBUp(pt,nFlags);
	CScrollView::OnLButtonUp(nFlags, point);
}


void CMainView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=GetMousePos(point,this);
	m_TreeList.OnMMove(pt,nFlags);
	CScrollView::OnMouseMove(nFlags, point);
}


void CMainView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=GetMousePos(point,this);
	m_TreeList.OnLBDblClick(pt,nFlags);
	CScrollView::OnLButtonDblClk(nFlags, point);
}


void CMainView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=GetMousePos(point,this);
	m_TreeList.OnRBDown(pt,nFlags);
	CScrollView::OnRButtonDown(nFlags, point);
}


void CMainView::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint pt=GetMousePos(point,this);
	m_TreeList.OnRBUp(pt,nFlags);
	CScrollView::OnRButtonUp(nFlags, point);
}


void CMainView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
	CPoint point=GetMousePos(this);
	m_TreeList.OnMMove(point,GetKey());
}


void CMainView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default
	CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
	CRect rect;
	m_TreeList.GetCanvasRect(&rect);
	SendMessageToIDWnd(IDW_HEAD_BAR,WM_SET_VIEW_SIZE,(WPARAM)&rect);
}


BOOL CMainView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	BOOL ret=CScrollView::OnMouseWheel(nFlags, zDelta, pt);
	CPoint point=GetMousePos(this);
	m_TreeList.OnMMove(point,GetKey());
	return ret;
}


BOOL CMainView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;//CScrollView::OnEraseBkgnd(pDC);
}


void CMainView::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);
	CRect rect;
	m_TreeList.GetCanvasRect(&rect);
	m_TreeList.SetScrollSizes(CSize(rect.Width(),-1));
	SendMessageToIDWnd(IDW_BASE_BAR,WM_SET_VIEW_SIZE,(WPARAM)&rect);
	SendMessageToIDWnd(IDW_HEAD_BAR,WM_SET_VIEW_SIZE,(WPARAM)&rect);
	// TODO: Add your message handler code here
}
