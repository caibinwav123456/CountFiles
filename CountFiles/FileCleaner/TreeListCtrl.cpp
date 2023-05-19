#include "pch.h"
#include "TreeListCtrl.h"
#include "resource.h"
#define t2a(p) ConvertTStrToAnsiStr(p)
#define a2t(p) ConvertAnsiStrToTStr(p)
enum E_FOLDER_STATE
{
	eFSEqual=1,
	eFSOld,
	eFSNew,
	eFSSolo,
	eFSNewOld,
	eFSSoloOld,
	eFSNewSolo,
	eFSNReady,
	eFSMax,
};
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
TreeListCtrl::TreeListCtrl(CWnd* pWnd):m_pWnd(pWnd),m_bExpand(FALSE)
{

}
TreeListCtrl::~TreeListCtrl()
{

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
	drawer->DrawBitmap(expand&&state!=eFSNReady?&m_bmpFolderExpMask:&m_bmpFolderMask,pt,SRCAND,
		&CRect(0,0,LINE_HEIGHT,LINE_HEIGHT));
	drawer->DrawBitmap(expand?&m_bmpFolderExp:&m_bmpFolder,pt,SRCPAINT,
		&CRect(LINE_HEIGHT*(state-1),0,LINE_HEIGHT*state,LINE_HEIGHT));
}
void TreeListCtrl::Draw(CDC* pClientDC,bool buffered)
{
	CDCDraw canvas(m_pWnd,pClientDC,buffered);
	CDrawer drawer(&canvas);
	drawer.FillEllipse(&CRect(500,500,600,600),RGB(255,0,255));
	drawer.DrawEllipse(&CRect(300,300,400,400),RGB(255,0,0),5);
	drawer.DrawRect(&CRect(400,200,500,300),RGB(0,0,255),3);
	drawer.FillRect(&CRect(200,400,300,500),RGB(0,255,0));
	//drawer.DrawBitmap(&m_bmpFolder,&CPoint(100,100));
	drawer.FillRect(&CRect(90,90,834,134),RGB(255,255,0));
	for(int i=eFSEqual;i<eFSMax;i++)
		DrawFolder(&drawer,&CPoint(100*i,100),i,m_bExpand);
	drawer.DrawText(&CPoint(200,200),_T("hello"),50,RGB(255,0,127));
}
void TreeListCtrl::OnLBDown(const CPoint& pt)
{

}
void TreeListCtrl::OnLBUp(const CPoint& pt)
{

}
void TreeListCtrl::OnLBDblClick(const CPoint& pt)
{
	m_bExpand=!m_bExpand;
	Invalidate();
}
void TreeListCtrl::OnRBDown(const CPoint& pt)
{

}
void TreeListCtrl::OnRBUp(const CPoint& pt)
{

}
void TreeListCtrl::OnMMove(const CPoint& pt)
{

}
