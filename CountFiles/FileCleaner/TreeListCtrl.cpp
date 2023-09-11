#include "pch.h"
#include "TreeListCtrl.h"
#include "utility.h"
#include "resource.h"
#include <assert.h>
COLORREF GetDispColor(E_FOLDER_STATE state)
{
	switch(state)
	{
	case eFSOld:
		return OLD_COLOR;
	case eFSNew:
	case eFSNewOld:
	case eFSNewSolo:
		return RED_COLOR;
	case eFSSolo:
	case eFSSoloOld:
		return BLUE_COLOR;
	case eFSNReady:
	case eFSError:
		return YELLOW_COLOR;
	default:
		return RGB(0,0,0);
	}
}
TreeListCtrl::TreeListCtrl(CWnd* pWnd):m_pWnd(pWnd),m_nTotalLine(0),m_pRootItem(NULL),m_ItemSel(this),m_iCurLine(-1)
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
	SetScrollSizes(CSize(-1,m_nTotalLine*LINE_HEIGHT));
	return 0;
failed:
	Exit();
	return -1;
}
void TreeListCtrl::Exit()
{
	UnLoad();
	m_bmpFolder.DeleteObject();
	m_bmpFolderMask.DeleteObject();
	m_bmpFolderExp.DeleteObject();
	m_bmpFolderExpMask.DeleteObject();
}
int TreeListCtrl::Load(const char* lfile,const char* efile)
{
	UnLoad();
	if(lfile==NULL||*lfile==0)
		return false;
	int ret=0;
	if(0!=(ret=m_ListLoader.Load(lfile,efile)))
		return ret;
	TLItemDir* pRoot=new TLItemDir(&m_ListLoader);
	m_pRootItem=pRoot;
	pRoot->type=eITypeDir;
	pRoot->dirnode=m_ListLoader.GetRootNode();
	if(0!=(ret=pRoot->OpenDir(true,false)))
	{
		UnLoad();
		return ret;
	}
	UpdateListStat();
	return 0;
}
void TreeListCtrl::UnLoad()
{
	m_ItemSel.SetSel(NULL,-1);
	if(m_pRootItem!=NULL)
	{
		TLItemDir* root=dynamic_cast<TLItemDir*>(m_pRootItem);
		if(root!=NULL)
		{
			root->Release();
			delete root;
		}
	}
	m_pRootItem=NULL;
	m_ListLoader.Unload();
	UpdateListStat();
}
void TreeListCtrl::Invalidate()
{
	m_pWnd->Invalidate(FALSE);
}
void TreeListCtrl::DrawFolder(CDrawer* drawer,POINT* pt,E_FOLDER_STATE state,BOOL expand)
{
	if(state<=0||state>=eFSMax)
		return;
	drawer->DrawBitmap(expand&&state<eFSAnormal?&m_bmpFolderExpMask:&m_bmpFolderMask,pt,SRCAND,
		&CRect(0,0,LINE_HEIGHT,LINE_HEIGHT));
	drawer->DrawBitmap(expand?&m_bmpFolderExp:&m_bmpFolder,pt,SRCPAINT,
		&CRect(LINE_HEIGHT*(state-1),0,LINE_HEIGHT*state,LINE_HEIGHT));
}
ListCtrlIterator TreeListCtrl::GetDrawIter(POINT* pt)
{
	POINT dummy,*ptrpt;
	ptrpt=(pt!=NULL?pt:&dummy);
	if(pt==NULL)
	{
		CRect rc;
		GetCanvasRect(&rc);
		dummy=rc.TopLeft();
	}
	return GetListIter(LineNumFromPt(ptrpt));
}
ListCtrlIterator TreeListCtrl::GetListIter(int iline)
{
	return ListCtrlIterator(m_pRootItem,iline,this);
}
int TreeListCtrl::LineNumFromPt(POINT* pt)
{
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
	if(m_ItemSel.IsSelected(pItem,iline))
		color=SEL_COLOR;
	drawer.FillRect(&rcline,color);
	if(pItem!=NULL&&m_ItemSel.IsFocus(iline))
		drawer.DrawRect(&rcline,RGB(0,0,0),1,PS_DOT);
}
void TreeListCtrl::DrawConn(CDrawer& drawer,const ListCtrlIterator& iter)
{
	if(iter.m_pStkItem==NULL)
		return;
	ItStkItem* pstk=iter.m_pStkItem;
	int level=iter.lvl;
	int ystart=iter.m_iline*LINE_HEIGHT;
	int yend=ystart+LINE_HEIGHT;
	int ypos=ystart+CONN_Y;
	for(;pstk->next!=NULL;pstk=pstk->next,level--)
	{
		assert(level>0);
		int xpos=LINE_INDENT*level+CONN_START;
		bool last=pstk->parentidx==(int)pstk->m_pLItem->parent->subitems.size()-1;
		if(level==iter.lvl)
		{
			int xposend=LINE_INDENT*level+CONN_END;
			drawer.DrawLine(&CPoint(xpos,ypos),&CPoint(xposend,ypos),CONN_COLOR,1,PS_DOT);
		}
		if(!last)
			drawer.DrawLine(&CPoint(xpos,ystart),&CPoint(xpos,yend),CONN_COLOR,1,PS_DOT);
		else if(level==iter.lvl)
			drawer.DrawLine(&CPoint(xpos,ystart),&CPoint(xpos,ypos),CONN_COLOR,1,PS_DOT);
	}
}
inline void DrawConfinedText(CDrawer& drawer,const string& text,const CRect& rc,COLORREF clr)
{
	CString txt=a2t(text);
	CSize txtsize=drawer.GetTextExtent(txt,TEXT_HEIGHT);
	if(txtsize.cx>rc.Width())
	{
		CString tmptxt=txt;
		while((!tmptxt.IsEmpty())&&txtsize.cx>rc.Width())
		{
			tmptxt=tmptxt.Left(tmptxt.GetLength()-1);
			txt=tmptxt+_T("...");
			txtsize=drawer.GetTextExtent(txt,TEXT_HEIGHT);
		}
		if(tmptxt.IsEmpty())
			txt.Empty();
	}
	CPoint pt(rc.left,rc.top+(rc.Height()-TEXT_HEIGHT)/2);
	drawer.DrawText(&pt,txt,TEXT_HEIGHT,clr);
}
void TreeListCtrl::DrawLine(CDrawer& drawer,const ListCtrlIterator& iter)
{
	DrawLine(drawer,iter.m_iline,iter.m_pStkItem==NULL?NULL:iter.m_pStkItem->m_pLItem);
	DrawConn(drawer,iter);
	CPoint pos(0,LINE_HEIGHT*iter.m_iline);
	pos.x+=LINE_INDENT*iter.lvl;
	TLItem* item=iter.m_pStkItem->m_pLItem;
	COLORREF clr=GetDispColor(item->state);
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
			DrawFolder(&drawer,&pos,item->state,exp);
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
			file_node_info info;
			string date;
			if(item->type==eITypeDir)
				m_ListLoader.GetNodeInfo(item->dirnode,&info);
			else
				m_ListLoader.GetNodeInfo(item->filenode,&info);
			info.mod_time.Format(date,FORMAT_DATE|FORMAT_TIME|FORMAT_WEEKDAY);
			DrawConfinedText(drawer,info.name,CRect(pos,CPoint(400,pos.y+LINE_HEIGHT)),clr);
			DrawConfinedText(drawer,format_segmented_u64(info.size),CRect(400,pos.y,500,pos.y+LINE_HEIGHT),clr);
			DrawConfinedText(drawer,date,CRect(500,pos.y,600,pos.y+LINE_HEIGHT),clr);
		}
		break;
	case eITypeErrDir:
	case eITypeErrFile:
		{
			err_node_info info;
			m_ListLoader.GetNodeErrInfo(item->errnode,&info);
			DrawConfinedText(drawer,info.name,CRect(pos,CPoint(400,pos.y+LINE_HEIGHT)),clr);
			DrawConfinedText(drawer,info.err_desc,CRect(400,pos.y,500,pos.y+LINE_HEIGHT),clr);
		}
		break;
	}
}
void TreeListCtrl::Draw(CDC* pClientDC,bool buffered)
{
	CDCDraw canvas(m_pWnd,pClientDC,buffered);
	CDrawer drawer(&canvas);
	for(ListCtrlIterator it=GetDrawIter();it;it++)
	{
		DrawLine(drawer,it);
	}
}
void TreeListCtrl::UpdateListStat()
{
	m_nTotalLine=(m_pRootItem==NULL?0:(m_pRootItem->GetDispLength()-1));
	SetScrollSizes(CSize(-1,m_nTotalLine*LINE_HEIGHT));
}
void TreeListCtrl::OnLBDown(const CPoint& pt,UINT nFlags)
{
	m_iCurLine=-1;
	int iline=LineNumFromPt((POINT*)&pt);
	if(!m_ItemSel.valid(iline))
		goto end;
	{
		ListCtrlIterator it=GetListIter(iline);
		if(it.m_pStkItem==NULL)
			goto end;
		TLItem* pItem=it.m_pStkItem->m_pLItem;
		if((nFlags|MK_CONTROL)&&(nFlags|MK_SHIFT))
		{
			m_ItemSel.CompoundSel(iline);
		}
		else if(nFlags|MK_CONTROL)
		{
			m_ItemSel.ToggleSel(pItem,iline);
		}
		else if(nFlags|MK_SHIFT)
		{
			m_ItemSel.ClearAndDragSel(pItem,iline);
		}
		else
		{
			m_ItemSel.SetSel(pItem,iline);
		}
		m_iCurLine=iline;
	}
end:
	Invalidate();
}
void TreeListCtrl::OnLBUp(const CPoint& pt,UINT nFlags)
{
	m_iCurLine=-1;
	Invalidate();
}
void TreeListCtrl::OnLBDblClick(const CPoint& pt,UINT nFlags)
{
	m_iCurLine=-1;
	int iline=LineNumFromPt((POINT*)&pt);
	if(!m_ItemSel.valid(iline))
		goto end;
	{
		ListCtrlIterator it=GetListIter(iline);
		if(it.m_pStkItem==NULL)
			goto end;
		TLItem* pItem=it.m_pStkItem->m_pLItem;
		assert(pItem!=NULL&&pItem->issel);
		if(pItem->type==eITypeDir)
		{
			m_ItemSel.SetSel(NULL,-1);
			TLItemDir* dir=dynamic_cast<TLItemDir*>(pItem);
			dir->OpenDir(!dir->isopen,false);
			m_ItemSel.SetSel(pItem,iline);
			UpdateListStat();
		}
	}
end:
	Invalidate();
}
void TreeListCtrl::OnRBDown(const CPoint& pt,UINT nFlags)
{
	m_ItemSel.EndDragSel();
	Invalidate();
}
void TreeListCtrl::OnRBUp(const CPoint& pt,UINT nFlags)
{
	m_ItemSel.EndDragSel();
	Invalidate();
}
void TreeListCtrl::OnMMove(const CPoint& pt,UINT nFlags)
{
	int iline=LineNumFromPt((POINT*)&pt);
	if((nFlags|MK_LBUTTON)&&m_ItemSel.valid(iline)&&m_ItemSel.valid(m_iCurLine)
		&&iline!=m_iCurLine)
	{
		m_ItemSel.DragSelTo(iline);
		m_iCurLine=iline;
		Invalidate();
	}
}
