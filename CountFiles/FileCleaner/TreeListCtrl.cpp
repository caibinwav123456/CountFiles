#include "pch.h"
#include "TreeListCtrl.h"
#include "resource.h"
string ConvertTStrToAnsiStr(LPCTSTR from)
{
	USES_CONVERSION;
	return T2A(from);
}
CString ConvertAnsiStrToTStr(LPCSTR from)
{
	USES_CONVERSION;
	return A2T(from);
}
CString ConvertAnsiStrToTStr(const string& from)
{
	USES_CONVERSION;
	return A2T(from.c_str());
}
TreeListCtrl::TreeListCtrl(CWnd* pWnd):m_pWnd(pWnd),m_nTotalLine(0),m_pRootItem(NULL),m_pItemSel(NULL)
{

}
TreeListCtrl::~TreeListCtrl()
{

}
inline void TreeListCtrl::GetCanvasRect(RECT* rc)
{
	m_pWnd->GetClientRect(rc);
	((CRect*)rc)->MoveToXY(GetScrollPos());
}
int TreeListCtrl::Init()
{
	if(!m_bmpFolder.LoadBitmap(IDB_FOLDER))
		goto failed;
	if(!m_bmpFolderMask.LoadBitmap(IDB_FOLDER_MASK))
		goto failed;
	if(!m_bmpFolderExp.LoadBitmap(IDB_FOLDER_EXP))
		goto failed;
	if(!m_bmpFolderExpMask.LoadBitmap(IDB_FOLDER_EXP_MASK))
		goto failed;
	SetScrollSizes(CSize(10*LINE_HEIGHT,LINE_HEIGHT));
	return 0;
failed:
	Exit();
	return -1;
}
void TreeListCtrl::Exit()
{
	m_bmpFolder.DeleteObject();
	m_bmpFolderMask.DeleteObject();
	m_bmpFolderExp.DeleteObject();
	m_bmpFolderExpMask.DeleteObject();
}
void TreeListCtrl::Invalidate()
{
	m_pWnd->Invalidate(FALSE);
}
void TreeListCtrl::DrawFolder(CDrawer* drawer,POINT* pt,int state,BOOL expand)
{
	if(state<=0||state>=eFSMax)
		return;
	drawer->DrawBitmap(expand&&state<eFSAnormal?&m_bmpFolderExpMask:&m_bmpFolderMask,pt,SRCAND,
		&CRect(0,0,LINE_HEIGHT,LINE_HEIGHT));
	drawer->DrawBitmap(expand?&m_bmpFolderExp:&m_bmpFolder,pt,SRCPAINT,
		&CRect(LINE_HEIGHT*(state-1),0,LINE_HEIGHT*state,LINE_HEIGHT));
}
ListCtrlDrawIterator TreeListCtrl::GetDrawIter()
{
	CRect rc;
	GetCanvasRect(&rc);
	ListCtrlDrawIterator it(this);
	it.m_iline=LineNumFromPt(&rc.TopLeft());
	return it;
}
int TreeListCtrl::LineNumFromPt(POINT* pt)
{
	CRect rc;
	GetCanvasRect(&rc);
	int iline;
	if(pt->x<0||pt->y<0)
		return -1;
	iline=pt->y/LINE_HEIGHT;
	if(iline>=(int)m_nTotalLine)
		return -1;
	return iline;
}
bool TreeListCtrl::EndOfDraw(int iline)
{
	if(iline<0||iline>=(int)m_nTotalLine)
		return true;
	CRect rc;
	GetCanvasRect(&rc);
	return iline*LINE_HEIGHT>rc.bottom;
}
void TreeListCtrl::DrawLine(CDrawer& drawer,int iline)
{
	if(iline<0)
		return;
	CRect rcline(0,0,0,LINE_HEIGHT),rc;
	GetCanvasRect(&rc);
	rcline.right=rc.right;
	int starty=iline*LINE_HEIGHT;
	rcline.MoveToXY(0,starty);
	bool grey=!!(iline%2);
	drawer.FillRect(&rcline,grey?GREY_COLOR:RGB(255,255,255));
}
void TreeListCtrl::DrawLine(CDrawer& drawer,const ListCtrlDrawIterator& iter)
{
	DrawLine(drawer,iter.m_iline);
}
void TreeListCtrl::Draw(CDC* pClientDC,bool buffered)
{
	CDCDraw canvas(m_pWnd,pClientDC,buffered);
	CDrawer drawer(&canvas);
	CRect rc;
	GetCanvasRect(&rc);
	for(ListCtrlDrawIterator it=GetDrawIter();it;it++)
	{
		DrawLine(drawer,it);
	}
}
void TreeListCtrl::OnLBDown(const CPoint& pt)
{
	Invalidate();
}
void TreeListCtrl::OnLBUp(const CPoint& pt)
{
	Invalidate();
}
void TreeListCtrl::OnLBDblClick(const CPoint& pt)
{
	Invalidate();
}
void TreeListCtrl::OnRBDown(const CPoint& pt)
{
	Invalidate();
}
void TreeListCtrl::OnRBUp(const CPoint& pt)
{
	Invalidate();
}
void TreeListCtrl::OnMMove(const CPoint& pt)
{

}
