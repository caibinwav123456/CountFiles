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
	SetScrollSizes(CSize(10*LINE_HEIGHT,m_nTotalLine*LINE_HEIGHT));
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
ListCtrlDrawIterator TreeListCtrl::GetDrawIter(POINT* pt)
{
	POINT dummy,*ptrpt;
	ptrpt=(pt!=NULL?pt:&dummy);
	if(pt==NULL)
	{
		CRect rc;
		GetCanvasRect(&rc);
		dummy=rc.TopLeft();
	}
	ListCtrlDrawIterator it(this);
	it.m_iline=LineNumFromPt(ptrpt);
	it.m_pStkItem=(m_pRootItem==NULL?NULL:m_pRootItem->FromLineNum(it.m_iline,it.lvl));
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
	return iline*LINE_HEIGHT>rc.bottom||(iline+1)*LINE_HEIGHT<rc.bottom;
}
void TreeListCtrl::DrawLine(CDrawer& drawer,int iline,TLItem* pItem)
{
	if(iline<0)
		return;
	CRect rcline(0,0,0,LINE_HEIGHT),rc;
	GetCanvasRect(&rc);
	rcline.right=rc.right;
	int starty=iline*LINE_HEIGHT;
	rcline.MoveToXY(0,starty);
	bool grey=!!(iline%2);
	COLORREF color=grey?GREY_COLOR:RGB(255,255,255);
	if(pItem!=NULL&&pItem->issel)
		color=SEL_COLOR;
	drawer.FillRect(&rcline,color);
	if(pItem!=NULL&&pItem==m_pItemSel)
		drawer.DrawRect(&rcline,RGB(0,0,0),1,PS_DOT);
}
void TreeListCtrl::DrawLine(CDrawer& drawer,const ListCtrlDrawIterator& iter)
{
	DrawLine(drawer,iter.m_iline,iter.m_pStkItem==NULL?NULL:iter.m_pStkItem->m_pLItem);
	CPoint pos(0,LINE_HEIGHT*iter.m_iline);
	pos.x+=LINE_INDENT*iter.lvl;
	TLItem* item=iter.m_pStkItem->m_pLItem;
	bool err=item->type==eITypeErrDir||item->type==eITypeErrFile;
	COLORREF clr=err?RGB(255,255,0):RGB(0,0,0);
	switch(item->type)
	{
	case eITypeNone:
		return;
	case eITypeDir:
	case eITypeErrDir:
		{
			BOOL exp=FALSE;
			if(item->type==eITypeDir)
			{
				TLItemDir* dir=dynamic_cast<TLItemDir*>(item);
				exp=(BOOL)dir->isopen;
			}
			DrawFolder(&drawer,&pos,err?eFSError:eFSEqual,exp);
			pos.x+=FOLDER_WIDTH;
		}
		break;
	case eITypeFile:
	case eITypeErrFile:
		{
			CRect rcFile=FILE_RECT+pos;
			drawer.FillRect(&rcFile,clr);
			pos.x+=FILE_WIDTH;
		}
		break;
	}
	switch(item->type)
	{
	case eITypeDir:
	case eITypeFile:
		{
			CString strName,strSize,strDate;
			file_node_info info;
			string date;
			if(item->type==eITypeDir)
				m_ListLoader.GetNodeInfo(item->dirnode,&info);
			else
				m_ListLoader.GetNodeInfo(item->filenode,&info);
			strName=a2t(info.name);
			strSize=a2t(FormatI64(info.size));
			info.mod_time.Format(date,FORMAT_DATE|FORMAT_TIME|FORMAT_WEEKDAY);
			strDate=a2t(date);
			CSize txtsize=drawer.GetTextExtent(strName,LINE_HEIGHT);
			drawer.DrawText(&pos,strName,LINE_HEIGHT,clr);
			pos.x+=txtsize.cx+300;
			txtsize=drawer.GetTextExtent(strSize,LINE_HEIGHT);
			drawer.DrawText(&pos,strSize,LINE_HEIGHT,clr);
			pos.x+=txtsize.cx+300;
			txtsize=drawer.GetTextExtent(strDate,LINE_HEIGHT);
			drawer.DrawText(&pos,strDate,LINE_HEIGHT,clr);
			pos.x+=txtsize.cx+300;
		}
		break;
	case eITypeErrDir:
	case eITypeErrFile:
		{
			CString strName,strErrDesc;
			err_node_info info;
			m_ListLoader.GetNodeErrInfo(item->errnode,&info);
			strName=a2t(info.name);
			strErrDesc=a2t(info.err_desc);
			CSize txtsize=drawer.GetTextExtent(strName,LINE_HEIGHT);
			drawer.DrawText(&pos,strName,LINE_HEIGHT,clr);
			pos.x+=txtsize.cx+300;
			txtsize=drawer.GetTextExtent(strErrDesc,LINE_HEIGHT);
			drawer.DrawText(&pos,strErrDesc,LINE_HEIGHT,clr);
			pos.x+=txtsize.cx+300;
		}
		break;
	}
}
void TreeListCtrl::Draw(CDC* pClientDC,bool buffered)
{
	CDCDraw canvas(m_pWnd,pClientDC,buffered);
	CDrawer drawer(&canvas);
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
