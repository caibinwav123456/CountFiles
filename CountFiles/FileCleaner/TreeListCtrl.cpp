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
void TreeListCtrl::SetTabInfo(const TreeListTabGrid& tab)
{
	m_tabInfo=tab;
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
		return 0;
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
	CPoint point=*(CPoint*)pt;
	CRect rcSrcMask,rcSrc;
	if(FOLDER_METRIC>LINE_HEIGHT)
	{
		rcSrcMask=CRect((FOLDER_METRIC-LINE_HEIGHT)/2,(FOLDER_METRIC-LINE_HEIGHT)/2,
			(FOLDER_METRIC+LINE_HEIGHT)/2,(FOLDER_METRIC+LINE_HEIGHT)/2);
		rcSrc=CRect(FOLDER_METRIC*(state-1)+(FOLDER_METRIC-LINE_HEIGHT)/2,(FOLDER_METRIC-LINE_HEIGHT)/2,
			FOLDER_METRIC*state-(FOLDER_METRIC-LINE_HEIGHT)/2,(FOLDER_METRIC+LINE_HEIGHT)/2);
	}
	else
	{
		point=point+CPoint((LINE_HEIGHT-FOLDER_METRIC)/2,(LINE_HEIGHT-FOLDER_METRIC)/2);
		rcSrcMask=CRect(0,0,FOLDER_METRIC,FOLDER_METRIC);
		rcSrc=CRect(FOLDER_METRIC*(state-1),0,FOLDER_METRIC*state,FOLDER_METRIC);
	}
	drawer->DrawBitmap(expand&&state<eFSAnormal?&m_bmpFolderExpMask:&m_bmpFolderMask,&point,SRCAND,
		&rcSrcMask);
	drawer->DrawBitmap(expand?&m_bmpFolderExp:&m_bmpFolder,&point,SRCPAINT,
		&rcSrc);
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
	return iline*LINE_HEIGHT>rc.bottom||(iline+1)*LINE_HEIGHT<rc.top;
}
void TreeListCtrl::DrawLine(CDrawer& drawer,int iline,TLItem* pItem)
{
	if(iline<0)
		return;
	CRect rcline(0,0,0,LINE_HEIGHT),rc;
	GetCanvasRect(&rc);
	rcline.right=max(rc.right,MIN_SCROLL_WIDTH);
	int starty=iline*LINE_HEIGHT;
	rcline.MoveToXY(0,starty);
	bool grey=!!(iline%2);
	COLORREF color=grey?GREY_COLOR:RGB(255,255,255);
	if(m_ItemSel.IsSelected(pItem,iline))
		color=SEL_COLOR;
	CRect rect=rcline;
	rect.InflateRect(&CRect(0,0,1,1));
	drawer.FillRect(&rect,color);
	rcline.DeflateRect(&CRect(0,0,1,1));
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
		int xpos=LINE_INDENT*(level-1)+CONN_START;
		bool last=pstk->parentidx==(int)pstk->m_pLItem->parent->subitems.size()-1;
		if(level==iter.lvl)
		{
			int xposend=LINE_INDENT*(level-1)+CONN_END;
			drawer.DrawLine(&CPoint(xpos,ypos),&CPoint(xposend,ypos),CONN_COLOR,1,PS_DOT);
		}
		if(!last)
			drawer.DrawLine(&CPoint(xpos,ystart),&CPoint(xpos,yend),CONN_COLOR,1,PS_DOT);
		else if(level==iter.lvl)
			drawer.DrawLine(&CPoint(xpos,ystart),&CPoint(xpos,ypos),CONN_COLOR,1,PS_DOT);
	}
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
			pos.x+=LINE_HEIGHT;
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
			string date,time;
			int tabidx=0;
			if(item->type==eITypeDir)
				m_ListLoader.GetNodeInfo(item->dirnode,&info);
			else
				m_ListLoader.GetNodeInfo(item->filenode,&info);
			info.mod_time.Format(date,FORMAT_DATE|FORMAT_WEEKDAY);
			info.mod_time.Format(time,FORMAT_TIME);
			ASSERT(m_tabInfo.mask&TLTAB_NAME);
			drawer.DrawText(&CRect(pos,CPoint(m_tabInfo.arrTab[tabidx++].rect.right,pos.y+LINE_HEIGHT)),DT_ALIGN_LEFT,
				a2t(info.name),TEXT_HEIGHT,clr,TRANSPARENT,VIEW_FONT);
			if(m_tabInfo.mask&TLTAB_SIZE)
			{
				drawer.DrawText(&CRect(m_tabInfo.arrTab[tabidx].rect.left,pos.y,
					m_tabInfo.arrTab[tabidx].rect.right-TABMARGIN_SIZE,pos.y+LINE_HEIGHT),DT_ALIGN_RIGHT,
					a2t(format_segmented_u64(info.size)),TEXT_HEIGHT,clr,TRANSPARENT,VIEW_FONT);
				tabidx++;
			}
			if(m_tabInfo.mask&TLTAB_MODIFY)
			{
				int length=min(m_tabInfo.arrTab[tabidx].rect.left+MODIFY_DATE_PART_WIDTH,m_tabInfo.arrTab[tabidx].rect.right);
				drawer.DrawText(&CRect(m_tabInfo.arrTab[tabidx].rect.left,pos.y,
					length,pos.y+LINE_HEIGHT),DT_ALIGN_LEFT,
					a2t(date),TEXT_HEIGHT,clr,TRANSPARENT,VIEW_FONT);
				drawer.DrawText(&CRect(m_tabInfo.arrTab[tabidx].rect.left+MODIFY_DATE_PART_WIDTH,pos.y,
					m_tabInfo.arrTab[tabidx].rect.right,pos.y+LINE_HEIGHT),DT_ALIGN_LEFT,
					a2t(time),TEXT_HEIGHT,clr,TRANSPARENT,VIEW_FONT);
				tabidx++;
			}
		}
		break;
	case eITypeErrDir:
	case eITypeErrFile:
		{
			err_node_info info;
			int tabidx=0;
			m_ListLoader.GetNodeErrInfo(item->errnode,&info);
			ASSERT(m_tabInfo.mask&TLTAB_NAME);
			drawer.DrawText(&CRect(pos,CPoint(m_tabInfo.arrTab[tabidx].rect.right,pos.y+LINE_HEIGHT)),DT_ALIGN_LEFT,
				a2t(info.name),TEXT_HEIGHT,clr,TRANSPARENT,VIEW_FONT);
			drawer.DrawText(&CRect(m_tabInfo.arrTab[tabidx].rect.right,pos.y,
				m_tabInfo.rcTotal.right,pos.y+LINE_HEIGHT),DT_ALIGN_LEFT,
				a2t(info.err_desc),TEXT_HEIGHT,clr,TRANSPARENT,VIEW_FONT);
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

	//Draw separator
	CRect rc;
	GetCanvasRect(&rc);
	int grplen=(max(rc.Width(),MIN_SCROLL_WIDTH)-BAR_CENTER_SPACE)/2;
	int top=max(0,rc.top);
	int bottom=min((int)m_nTotalLine*LINE_HEIGHT,rc.bottom);
	drawer.DrawLine(&CPoint(grplen,top),&CPoint(grplen,bottom),BACK_GREY_COLOR,1,PS_SOLID);
	drawer.DrawLine(&CPoint(grplen+BAR_CENTER_SPACE,top),&CPoint(grplen+BAR_CENTER_SPACE,bottom),
		BACK_GREY_COLOR,1,PS_SOLID);
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
		if((nFlags&MK_CONTROL)&&(nFlags&MK_SHIFT))
		{
			m_ItemSel.CompoundSel(iline);
		}
		else if(nFlags&MK_CONTROL)
		{
			m_ItemSel.ToggleSel(pItem,iline);
		}
		else if(nFlags&MK_SHIFT)
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
	if((nFlags&MK_LBUTTON)&&m_ItemSel.valid(iline)&&m_ItemSel.valid(m_iCurLine)
		&&iline!=m_iCurLine)
	{
		m_ItemSel.DragSelTo(iline);
		m_iCurLine=iline;
		Invalidate();
	}
}
