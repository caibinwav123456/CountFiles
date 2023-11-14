#include "pch.h"
#include "TreeListCtrl.h"
#include "utility.h"
#include "resource.h"
#include <assert.h>
COLORREF __get_disp_color__(uint state)
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
COLORREF GetDispColor(E_FOLDER_STATE state)
{
	static struct ClrArray
	{
		COLORREF arr[eFSMax];
		ClrArray()
		{
			for(uint i=0;i<eFSMax;i++)
				arr[i]=__get_disp_color__(i);
		}
	}clr_array;
	return clr_array.arr[state];
}
TreeListCtrl::TreeListCtrl(CWnd* pWnd):m_pWnd(pWnd),m_iCurLine(-1)
{
	m_vecLists.push_back(new TLUnit(this,&m_tabLeft,&m_tabRight));
	m_iVec=0;
	m_pCurTlU=m_vecLists[m_iVec];
}
TreeListCtrl::~TreeListCtrl()
{
	for(int i=0;i<(int)m_vecLists.size();i++)
		delete m_vecLists[i];
}
inline void TreeListCtrl::GetCanvasRect(RECT* rc)
{
	m_pWnd->GetClientRect(rc);
	((CRect*)rc)->MoveToXY(GetScrollPos());
}
void TreeListCtrl::SetTabInfo(const TabInfo* tab)
{
	m_tabLeft=*tab->left;
	m_tabRight=*tab->right;
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
	SetScrollSizes(CSize(-1,m_TlU.m_nTotalLine*LINE_HEIGHT));
	return 0;
failed:
	Exit();
	return -1;
}
void TreeListCtrl::Exit()
{
	UnLoad(true);
	m_bmpFolder.DeleteObject();
	m_bmpFolderMask.DeleteObject();
	m_bmpFolderExp.DeleteObject();
	m_bmpFolderExpMask.DeleteObject();
}
int TLUnit::InitialExpand()
{
	TLCore* pBase=GetPrimaryBase();
	assert(pBase!=NULL&&pBase->m_pBaseItem!=NULL);
	return pBase->m_pBaseItem->OpenDir(true,false);
}
TLCore* TLUnit::GetPrimaryBase(int* side)
{
	int dummy;
	if(side==NULL)
		side=&dummy;
	if(m_pItemJoint!=NULL)
	{
		*side=0;
		return &m_treeLeft;
	}
	else if(m_treeLeft.m_pBaseItem!=NULL)
	{
		*side=-1;
		return &m_treeLeft;
	}
	else if(m_treeRight.m_pBaseItem!=NULL)
	{
		*side=1;
		return &m_treeRight;
	}
	else
	{
		*side=0;
		return NULL;
	}
}
int TLUnit::LoadCore(TLCore& core,const char* lfile,const char* efile)
{
	int ret=0;
	return_ret(ret,0,core.m_ListLoader.Load(lfile,efile));
	TLItemDir* pRoot=new TLItemDir(&core);
	core.m_pRootItem=core.m_pBaseItem=pRoot;
	pRoot->type=eITypeDir;
	pRoot->dirnode=core.m_ListLoader.GetRootNode();
	return 0;
}
void TLUnit::UnLoadCore(TLCore& core)
{
	if(core.m_pBaseItem!=NULL)
		core.m_pBaseItem->parent=core.m_pBaseParent;
	if(core.m_pRootItem!=NULL)
	{
		core.m_pRootItem->Detach();
		core.m_pRootItem->Release();
		delete core.m_pRootItem;
	}
	core.m_pBaseItem=core.m_pRootItem=NULL;
	core.m_ListLoader.Unload();
}
int TLUnit::Load(UINT mask,const char* lfile,const char* efile,
	const char* lfileref,const char* efileref)
{
	UnLoad();
	if((lfile==NULL||*lfile==0)&&(lfileref==NULL||*lfileref==0))
		return 0;
	int ret=0;
	bool loadleft=(mask&FILE_LIST_ATTRIB_MAIN),
		loadright=(mask&FILE_LIST_ATTRIB_REF);
	assert(loadleft||loadright);
	if(loadleft&&lfile!=NULL&&*lfile!=0)
		fail_goto(ret,0,LoadCore(m_treeLeft,lfile,efile),fail);
	if(loadright&&lfileref!=NULL&&*lfileref!=0)
		fail_goto(ret,0,LoadCore(m_treeRight,lfileref,efileref),fail);
	if(loadleft&&loadright)
	{
		m_pItemJoint=new TLItemDir(NULL);
		m_treeLeft.m_pBaseItem->subpairs=
			m_treeRight.m_pBaseItem->subpairs=new TLItemSplice;
		TLItemSplice* splice=new TLItemSplice;
		splice->map.push_back(TLItemPair(m_treeLeft.m_pBaseItem,m_treeRight.m_pBaseItem));
		splice->jntitems.push_back(&splice->map[0]);
		m_pItemJoint->subpairs=splice;
		m_treeLeft.m_pBaseItem->parent=
			m_treeRight.m_pBaseItem->parent=m_pItemJoint;
		m_treeLeft.m_pBaseItem->parentidx=
			m_treeRight.m_pBaseItem->parentidx=0;
	}
	fail_goto(ret,0,InitialExpand(),fail);
	return 0;
fail:
	UnLoad();
	return ret;
}
void TLUnit::UnLoad()
{
	UnLoadCore(m_treeLeft);
	UnLoadCore(m_treeRight);
	if(m_pItemJoint!=NULL)
	{
		assert(m_pItemJoint->subpairs!=NULL);
		delete m_pItemJoint->subpairs;
		delete m_pItemJoint;
		m_pItemJoint=NULL;
	}
}
int TreeListCtrl::Load(UINT mask,const char* lfile,const char* efile,
	const char* lfileref,const char* efileref)
{
	m_iCurLine=-1;
	int ret=m_TlU.Load(mask,lfile,efile,lfileref,efileref);
	UpdateListStat();
	return ret;
}
void TreeListCtrl::UnLoad(bool bAll)
{
	m_iCurLine=-1;
	if(!bAll)
	{
		m_TlU.m_ItemSel.SetSel(NULL,-1);
		m_TlU.UnLoad();
	}
	else
	{
		for(int i=0;i<(int)m_vecLists.size();i++)
		{
			m_vecLists[i]->m_ItemSel.SetSel(NULL,-1);
			m_vecLists[i]->UnLoad();
		}
	}
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
	int side;
	TLCore* pCore=m_TlU.GetPrimaryBase(&side);
	return ListCtrlIterator(pCore==NULL?NULL:pCore->m_pBaseItem,iline,side,this);
}
int TreeListCtrl::LineNumFromPt(POINT* pt)
{
	int iline;
	if(pt->x<0||pt->y<0)
		return -1;
	iline=pt->y/LINE_HEIGHT;
	if(iline>=(int)m_TlU.m_nTotalLine)
		return -1;
	return iline;
}
bool TreeListCtrl::EndOfDraw(int iline)
{
	if(iline<0||iline>=(int)m_TlU.m_nTotalLine)
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
	if(m_TlU.m_ItemSel.IsSelected(pItem,iline))
		color=SEL_COLOR;
	CRect rect=rcline;
	rect.InflateRect(&CRect(0,0,1,1));
	drawer.FillRect(&rect,color);
	rcline.DeflateRect(&CRect(0,0,1,1));
	if(pItem!=NULL&&m_TlU.m_ItemSel.IsFocus(iline))
		drawer.DrawRect(&rcline,RGB(0,0,0),1,PS_DOT);
}
void TreeListCtrl::DrawConn(CDrawer& drawer,const ListCtrlIterator& iter,TLItem* item,int side,int xbase,ConnBuffer& conbuf)
{
	if(iter.m_pStkItem==NULL)
		return;
	vector<int>& buf=side<=0?conbuf.xleft:conbuf.xright;
	ItStkItem* pstk=iter.m_pStkItem;
	int level=iter.lvl;
	int ystart=iter.m_iline*LINE_HEIGHT;
	int yend=ystart+LINE_HEIGHT;
	int ypos=ystart+CONN_Y;
	if(item==NULL)
	{
		for(int i=0;i<(int)buf.size();i++)
			drawer.DrawLine(&CPoint(buf[i],ystart),&CPoint(buf[i],yend),CONN_COLOR,1,PS_DOT);
		return;
	}
	buf.clear();
	for(;pstk->next!=NULL;pstk=pstk->next,level--)
	{
		assert(level>0);
		int xpos=xbase+LINE_INDENT*(level-1)+CONN_START;
		TLItem* sitem=pstk->get_item(side);
		assert(sitem!=NULL);
		bool last=(sitem==sitem->parent->subitems.back());
		if(level==iter.lvl)
		{
			int xposend=LINE_INDENT*(level-1)+CONN_END;
			drawer.DrawLine(&CPoint(xpos,ypos),&CPoint(xposend,ypos),CONN_COLOR,1,PS_DOT);
		}
		if(!last)
		{
			drawer.DrawLine(&CPoint(xpos,ystart),&CPoint(xpos,yend),CONN_COLOR,1,PS_DOT);
			buf.push_back(xpos);
		}
		else if(level==iter.lvl)
			drawer.DrawLine(&CPoint(xpos,ystart),&CPoint(xpos,ypos),CONN_COLOR,1,PS_DOT);
	}
}
void TreeListCtrl::DrawLine(CDrawer& drawer,const ListCtrlIterator& iter,ConnBuffer& conbuf)
{
	DrawLine(drawer,iter.m_iline,iter.m_pStkItem==NULL?NULL:iter.m_pStkItem->m_pItem);
	DrawLineGrp(drawer,iter,m_TlU.m_treeLeft,conbuf);
	DrawLineGrp(drawer,iter,m_TlU.m_treeRight,conbuf);
}
void TreeListCtrl::DrawLineGrp(CDrawer& drawer,const ListCtrlIterator& iter,TLCore& tltree,ConnBuffer& conbuf)
{
	TreeListTabGrid& tab=*tltree.m_pTab;
	assert(tab.mask&TLTAB_NAME);
	CPoint pos(tab.rcTotal.left,LINE_HEIGHT*iter.m_iline);
	int side=(&tltree==&tltree.m_pTlUnit->m_treeLeft?-1:1);
	TLItem* item=iter.m_pStkItem->get_item(side);
	drawer.SetClipRect(&CRect(pos,CPoint(tab.arrTab[0].rect.right,pos.y+LINE_HEIGHT)));
	DrawConn(drawer,iter,item,side,tab.rcTotal.left,conbuf);
	if(item==NULL)
	{
		drawer.SetClipRect(NULL);
		return;
	}
	pos.x+=LINE_INDENT*iter.lvl;
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
			pos.x+=LINE_INDENT;
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
	drawer.SetClipRect(NULL);
	switch(item->type)
	{
	case eITypeDir:
	case eITypeFile:
		{
			file_node_info info;
			string date,time;
			int tabidx=0;
			if(item->type==eITypeDir)
				tltree.m_ListLoader.GetNodeInfo(item->dirnode,&info);
			else
				tltree.m_ListLoader.GetNodeInfo(item->filenode,&info);
			info.mod_time.Format(date,FORMAT_DATE|FORMAT_WEEKDAY);
			info.mod_time.Format(time,FORMAT_TIME);
			drawer.DrawText(&CRect(pos,CPoint(tab.arrTab[tabidx++].rect.right,pos.y+LINE_HEIGHT)),
				DT_ALIGN_LEFT,a2t(info.name),TEXT_HEIGHT,clr,TRANSPARENT,VIEW_FONT);
			if(tab.mask&TLTAB_SIZE)
			{
				drawer.DrawText(&CRect(tab.arrTab[tabidx].rect.left,pos.y,
					tab.arrTab[tabidx].rect.right-TABMARGIN_SIZE,pos.y+LINE_HEIGHT),DT_ALIGN_RIGHT,
					a2t(format_segmented_u64(info.size)),TEXT_HEIGHT,clr,TRANSPARENT,VIEW_FONT);
				tabidx++;
			}
			if(tab.mask&TLTAB_MODIFY)
			{
				int length=min(tab.arrTab[tabidx].rect.left+MODIFY_DATE_PART_WIDTH,
					tab.arrTab[tabidx].rect.right);
				drawer.DrawText(&CRect(tab.arrTab[tabidx].rect.left,pos.y,
					length,pos.y+LINE_HEIGHT),DT_ALIGN_LEFT,
					a2t(date),TEXT_HEIGHT,clr,TRANSPARENT,VIEW_FONT);
				drawer.DrawText(&CRect(tab.arrTab[tabidx].rect.left+MODIFY_DATE_PART_WIDTH,pos.y,
					tab.arrTab[tabidx].rect.right,pos.y+LINE_HEIGHT),DT_ALIGN_LEFT,
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
			tltree.m_ListLoader.GetNodeErrInfo(item->errnode,&info);
			drawer.DrawText(&CRect(pos,CPoint(tab.arrTab[tabidx].rect.right,pos.y+LINE_HEIGHT)),
				DT_ALIGN_LEFT,a2t(info.name),TEXT_HEIGHT,clr,TRANSPARENT,VIEW_FONT);
			drawer.DrawText(&CRect(tab.arrTab[tabidx].rect.right,pos.y,
				tab.rcTotal.right,pos.y+LINE_HEIGHT),DT_ALIGN_LEFT,
				a2t(info.err_desc),TEXT_HEIGHT,clr,TRANSPARENT,VIEW_FONT);
		}
		break;
	}
}
void TreeListCtrl::Draw(CDC* pClientDC,bool buffered)
{
	CDCDraw canvas(m_pWnd,pClientDC,buffered);
	CDrawer drawer(&canvas);
	ConnBuffer conbuf;
	for(ListCtrlIterator it=GetDrawIter();it;it++)
	{
		DrawLine(drawer,it,conbuf);
	}

	//Draw separator
	CRect rc;
	GetCanvasRect(&rc);
	int grplen=(max(rc.Width(),MIN_SCROLL_WIDTH)-BAR_CENTER_SPACE)/2;
	int top=max(0,rc.top);
	int bottom=min((int)m_TlU.m_nTotalLine*LINE_HEIGHT,rc.bottom);
	drawer.DrawLine(&CPoint(grplen,top),&CPoint(grplen,bottom),BACK_GREY_COLOR,1,PS_SOLID);
	drawer.DrawLine(&CPoint(grplen+BAR_CENTER_SPACE,top),&CPoint(grplen+BAR_CENTER_SPACE,bottom),
		BACK_GREY_COLOR,1,PS_SOLID);
}
void TreeListCtrl::UpdateListStat()
{
	TLCore* pBase=m_TlU.GetPrimaryBase();
	m_TlU.m_nTotalLine=(pBase==NULL||pBase->m_pBaseItem==NULL?
		0:(pBase->m_pBaseItem->GetDispLength()-1));
	SetScrollSizes(CSize(-1,m_TlU.m_nTotalLine*LINE_HEIGHT));
}
void TreeListCtrl::OnLBDown(const CPoint& pt,UINT nFlags)
{
	m_iCurLine=-1;
	int iline=LineNumFromPt((POINT*)&pt);
	if(!m_TlU.m_ItemSel.valid(iline))
		goto end;
	{
		ListCtrlIterator it=GetListIter(iline);
		if(it.m_pStkItem==NULL)
			goto end;
		if((nFlags&MK_CONTROL)&&(nFlags&MK_SHIFT))
		{
			m_TlU.m_ItemSel.CompoundSel(iline);
		}
		else if(nFlags&MK_CONTROL)
		{
			m_TlU.m_ItemSel.ToggleSel(it.m_pStkItem,iline);
		}
		else if(nFlags&MK_SHIFT)
		{
			m_TlU.m_ItemSel.ClearAndDragSel(it.m_pStkItem,iline);
		}
		else
		{
			m_TlU.m_ItemSel.SetSel(it.m_pStkItem,iline);
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
	if(!m_TlU.m_ItemSel.valid(iline))
		goto end;
	{
		ListCtrlIterator it=GetListIter(iline);
		if(it.m_pStkItem==NULL)
			goto end;
		TLItem* pItem=it.m_pStkItem->m_pItem;
		assert(pItem!=NULL&&pItem->issel);
		if(pItem->type==eITypeDir)
		{
			m_TlU.m_ItemSel.SetSel(NULL,-1);
			TLItemDir* dir=dynamic_cast<TLItemDir*>(pItem);
			dir->OpenDir(!dir->isopen,false);
			m_TlU.m_ItemSel.SetSel(it.m_pStkItem,iline);
			UpdateListStat();
		}
	}
end:
	Invalidate();
}
void TreeListCtrl::OnRBDown(const CPoint& pt,UINT nFlags)
{
	m_TlU.m_ItemSel.EndDragSel();
	Invalidate();
}
void TreeListCtrl::OnRBUp(const CPoint& pt,UINT nFlags)
{
	m_TlU.m_ItemSel.EndDragSel();
	Invalidate();
}
void TreeListCtrl::OnMMove(const CPoint& pt,UINT nFlags)
{
	int iline=LineNumFromPt((POINT*)&pt);
	if((nFlags&MK_LBUTTON)&&m_TlU.m_ItemSel.valid(iline)&&m_TlU.m_ItemSel.valid(m_iCurLine)
		&&iline!=m_iCurLine)
	{
		m_TlU.m_ItemSel.DragSelTo(iline);
		m_iCurLine=iline;
		Invalidate();
	}
}
