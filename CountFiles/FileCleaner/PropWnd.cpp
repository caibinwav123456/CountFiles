#include "pch.h"
#include "PropWnd.h"
#include "defines.h"
#include "resource.h"
IMPLEMENT_DYNAMIC(CPropWnd,CWnd)
IMPLEMENT_ID2WND_MAP(CPropWnd,IDW_PROP_WND)
void PropTabData::LinkAfter(PropTabData* data)
{
	PropTabData* nextdata=data->next;
	prev=data;
	next=nextdata;
	data->next=nextdata->prev=this;
}
void PropTabData::Remove()
{
	prev->next=next;
	next->prev=prev;
	prev=next=this;
}
PropTabData* PropTabStat::GetSelTab()
{
	return first.next==&last?NULL:first.next;
}
void PropTabStat::NewTab(const CString& left,const CString& right)
{
	PropTabData* data=new PropTabData;
	PropTabData* cursel=GetSelTab();
	data->left=left,data->right=right;
	int index;
	for(index=0;;index++)
	{
		if(indices.find(index)==indices.end())
			break;
	}
	data->ctrl_idx=(int)vecData.size();
	data->tab_idx=index;
	data->issel=true;
	if(cursel!=NULL)
		cursel->issel=false;
	vecData.push_back(data);
	indices.insert(index);
	data->LinkAfter(&first);
}
int PropTabStat::DeleteTab(int idx,int& next)
{
	if(idx<0||vecData.size()<=1||idx>=(int)vecData.size())
	{
		next=-1;
		return -1;
	}
	PropTabData* del=vecData[idx];
	PropTabData* cursel=GetSelTab();
	int mapidx=del->ctrl_idx;
	for(int i=0;i<(int)vecData.size();i++)
	{
		if(vecData[i]->ctrl_idx>mapidx)
			vecData[i]->ctrl_idx--;
	}
	if(del==cursel)
	{
		cursel->next->issel=true;
		next=cursel->next->ctrl_idx;
	}
	else
		next=-1;
	del->Remove();
	indices.erase(del->tab_idx);
	vecData.erase(vecData.begin()+idx);
	delete del;
	return mapidx;
}
bool PropTabStat::ReorderTab(int origin,int insert_before)
{
	if(origin<0||insert_before<0
		||origin>=(int)vecData.size()||insert_before>(int)vecData.size()-1)
		return false;
	PropTabData* data=vecData[origin];
	vecData.erase(vecData.begin()+origin);
	vecData.insert(vecData.begin()+insert_before,data);
	return true;
}
int PropTabStat::SelectTab(int idx,bool& unchanged)
{
	if(idx<0||idx>=(int)vecData.size())
		return -1;
	PropTabData* data=vecData[idx];
	PropTabData* cursel=GetSelTab();
	if(data==cursel)
	{
		unchanged=true;
		return cursel->ctrl_idx;
	}
	if(cursel!=NULL)
		cursel->issel=false;
	data->issel=true;
	data->Remove();
	data->LinkAfter(&first);
	unchanged=false;
	return data->ctrl_idx;
}
void PropTabStat::SetCurTabString(const CString& left,const CString& right)
{
	PropTabData* cursel=GetSelTab();
	if(cursel!=NULL)
		cursel->left=left,cursel->right=right;
}
CPropWnd::CPropWnd():m_iBaseX(PROP_START_X),m_nGrabIndex(-1),m_eGType(eGrabNone)
{

}
CPropWnd::~CPropWnd()
{

}
void CPropWnd::GetBitmapList(BitmapLoadInfo** blist,int* num)
{
	BitmapLoadInfo bitmaps[]={
		{&m_bmpBk,IDB_BMP_PROP_BACK},
		{&m_bmpTab,IDB_BMP_PROP},
		{&m_bmpTabMask,IDB_BMP_PROP_MASK},
		{&m_bmpTabSel,IDB_BMP_PROP_SEL},
		{&m_bmpTabSelMask,IDB_BMP_PROP_SEL_MASK},
		{&m_bmpTabBtnN,IDB_BMP_PROP_BTN_N},
		{&m_bmpTabBtnH,IDB_BMP_PROP_BTN_H},
		{&m_bmpTabBtnC,IDB_BMP_PROP_BTN_C},
		{&m_bmpTabBtnI,IDB_BMP_PROP_BTN_I},
	};
	*num=sizeof(bitmaps)/sizeof(BitmapLoadInfo);
	*blist=new BitmapLoadInfo[*num];
	memcpy(*blist,bitmaps,sizeof(bitmaps));
}
void CPropWnd::DeleteBitmaps()
{
	BitmapLoadInfo* bitmaps;
	int nbitmap;
	GetBitmapList(&bitmaps,&nbitmap);
	for(int i=0;i<nbitmap;i++)
	{
		bitmaps[i].bmp->DeleteObject();
	}
	delete[] bitmaps;
}
void CPropWnd::DrawTab(CDrawer& drawer,PropTabData* tab,int xpos)
{
	E_PROP_BTN_STATE state=tab->issel?ePropBtnNormal:ePropBtnDisable;
	if(m_nGrabIndex>=0&&m_PropStat.vecData[m_nGrabIndex]==tab)
	{
		switch(m_eGType)
		{
		case eGrabHoverBtn:
			state=ePropBtnHigh;
			break;
		case eGrabCloseBtn:
			state=ePropBtnClick;
			break;
		}
	}
	DrawTab(drawer,xpos,tab->tab_idx,tab->left,tab->right,tab->issel,state);
}
static inline CRect GetBitmapSize(CBitmap* bmp)
{
	BITMAP bm;
	bmp->GetBitmap(&bm);
	return CRect(0,0,bm.bmWidth,bm.bmHeight);
}
void CPropWnd::DrawTab(CDrawer& drawer,int xpos,int tabidx,const CString& left,const CString& right,bool sel,E_PROP_BTN_STATE state)
{
	CBitmap *bmpTab,*bmpMask,*btn;
	if(sel)
	{
		bmpTab=&m_bmpTabSel;
		bmpMask=&m_bmpTabSelMask;
	}
	else
	{
		bmpTab=&m_bmpTab;
		bmpMask=&m_bmpTabMask;
	}
	switch(state)
	{
	case ePropBtnNormal:
		btn=&m_bmpTabBtnN;
		break;
	case ePropBtnHigh:
		btn=&m_bmpTabBtnH;
		break;
	case ePropBtnClick:
		btn=&m_bmpTabBtnC;
		break;
	case ePropBtnDisable:
		btn=&m_bmpTabBtnI;
		break;
	default:
		btn=&m_bmpTabBtnN;
		break;
	}
	CRect rcTab(PAGE_TAB_RECT+CPoint(xpos,0)),
		rcString(PAGE_STRING_RECT+CPoint(xpos,0)),
		rcBtn(PROP_BUTTON_RECT+CPoint(xpos,0));

	CRect rcTabMargin(CPoint(rcTab.right-PAGE_TAB_MARGIN_SCALED,rcTab.top),rcTab.BottomRight());
	rcTab.DeflateRect(0,0,PAGE_TAB_MARGIN_SCALED,0);

	CRect rcSrc=GetBitmapSize(bmpTab);
	CRect rcSrcMargin(CPoint(rcSrc.right-PAGE_TAB_MARGIN,rcSrc.top),rcSrc.BottomRight());
	rcSrc.DeflateRect(0,0,PAGE_TAB_MARGIN,0);

	drawer.DrawBitmapScaled(bmpMask,rcTab,rcSrc,SRCAND);
	drawer.DrawBitmapScaled(bmpMask,rcTabMargin,rcSrcMargin,SRCAND);
	drawer.DrawBitmapScaled(bmpTab,rcTab,rcSrc,SRCPAINT);
	drawer.DrawBitmapScaled(bmpTab,rcTabMargin,rcSrcMargin,SRCPAINT);

	drawer.FillRect(rcBtn,RGB(0,0,0));
	drawer.DrawBitmapScaled(btn,rcBtn,NULL,SRCPAINT);

	if(left==_T("")&&right==_T(""))
	{
		CString strNewTab,strTabNew(_T("New Tab"));
		if(tabidx==0)
			strNewTab=strTabNew;
		else
			strNewTab.Format(_T("%s(%d)"),(LPCTSTR)strTabNew,tabidx);
		drawer.DrawText(rcString,DT_ALIGN_LEFT,strNewTab,TEXT_HEIGHT,RGB(0,0,0),TRANSPARENT,VIEW_FONT);
		return;
	}
	CString strDisp;
	if(left==_T("")||left==right)
		strDisp=right;
	else if(right==_T(""))
		strDisp=left;
	if(strDisp!=_T(""))
	{
		drawer.DrawText(rcString,DT_ALIGN_LEFT,strDisp,TEXT_HEIGHT,RGB(0,0,0),TRANSPARENT,VIEW_FONT);
		return;
	}
	CString sep=_T("<->");
	CSize sz=drawer.GetTextExtent(sep,TEXT_HEIGHT,VIEW_FONT),
		szLeft=drawer.GetTextExtent(left,TEXT_HEIGHT,VIEW_FONT),
		szRight=drawer.GetTextExtent(right,TEXT_HEIGHT,VIEW_FONT);
	if(sz.cx+szLeft.cx+szRight.cx<=rcString.Width())
	{
		CString strMingle;
		strMingle.Format(_T("%s%s%s"),(LPCTSTR)left,(LPCTSTR)sep,(LPCTSTR)right);
		drawer.DrawText(rcString,DT_ALIGN_LEFT,strMingle,TEXT_HEIGHT,RGB(0,0,0),TRANSPARENT,VIEW_FONT);
		return;
	}
	int width=(rcString.Width()-sz.cx)/2;
	if(width>szLeft.cx)
		width=szLeft.cx;
	CRect rcLeft(rcString.TopLeft(),CPoint(rcString.left+width,rcString.bottom)),
		rcRight(CPoint(rcString.right-width,rcString.top),rcString.BottomRight());
	CRect rcMid(rcLeft.right,rcLeft.top,rcRight.left,rcRight.bottom);
	drawer.DrawText(rcLeft,DT_ALIGN_LEFT,left,TEXT_HEIGHT,RGB(0,0,0),TRANSPARENT,VIEW_FONT);
	drawer.DrawText(rcMid,DT_ALIGN_LEFT,sep,TEXT_HEIGHT,RGB(0,0,0),TRANSPARENT,VIEW_FONT);
	drawer.DrawText(rcRight,DT_ALIGN_LEFT,right,TEXT_HEIGHT,RGB(0,0,0),TRANSPARENT,VIEW_FONT);
}
int CPropWnd::DetectGrabState(LPPOINT pt,bool mousedown,E_PROP_GRAB_TYPE& type)
{
	CPoint& point=*(CPoint*)pt;
	int x=point.x-m_iBaseX;
	if(x<0)
	{
		type=eGrabNone;
		return -1;
	}
	int idx=x/PAGE_TAB_WIDTH;
	if(idx>=(int)m_PropStat.vecData.size())
	{
		type=eGrabNone;
		return -1;
	}
	CPoint relative_coord=point-CSize(idx*PAGE_TAB_WIDTH,0);
	if(PROP_BUTTON_RECT.PtInRect(relative_coord))
		type=(mousedown?eGrabCloseBtn:eGrabHoverBtn);
	else
		type=eGrabTab;
	return idx;
}

BEGIN_MESSAGE_MAP(CPropWnd, CWnd)
	ON_MESSAGE(WM_SIZEPARENT, &CPropWnd::OnSizeParent)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_COMMAND(ID_FILE_NEW_TAB, &CPropWnd::OnFileNewTab)
END_MESSAGE_MAP()


int CPropWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	BitmapLoadInfo* bitmaps;
	int nbitmap;
	GetBitmapList(&bitmaps,&nbitmap);
	for(int i=0;i<nbitmap;i++)
	{
		if(!bitmaps[i].bmp->LoadBitmap(bitmaps[i].nIDResource))
		{
			delete[] bitmaps;
			DeleteBitmaps();
			return -1;
		}
	}
	delete[] bitmaps;
	m_PropStat.NewTab();
	return 0;
}


BOOL CPropWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}


void CPropWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	delete this;
}

LRESULT CPropWnd::OnSizeParent(WPARAM wParam, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;
	CRect rect(0,0,0,PROP_WND_HEIGHT),rcParent;
	GetParent()->GetClientRect(&rcParent);
	rect.right=rcParent.right;
	AfxRepositionWindow(lpLayout,m_hWnd,&rect);
	lpLayout->rect.top+=rect.Height();
	lpLayout->sizeTotal=CRect(lpLayout->rect).Size();
	return 0;
}


void CPropWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CWnd::OnPaint() for painting messages
	CRect rcClient;
	GetClientRect(&rcClient);
	CDCDraw canvas(this,&dc,true);
	CDrawer drawer(&canvas);
	drawer.DrawBitmapScaled(&m_bmpBk,&rcClient,NULL,SRCCOPY);
	int idxSel=-1;
	for(int i=0;i<(int)m_PropStat.vecData.size();i++)
	{
		if(!m_PropStat.vecData[i]->issel)
			DrawTab(drawer,m_PropStat.vecData[i],m_iBaseX+i*PAGE_TAB_WIDTH);
		else
			idxSel=i;
	}
	if(idxSel>=0)
		DrawTab(drawer,m_PropStat.vecData[idxSel],m_iBaseX+idxSel*PAGE_TAB_WIDTH);
}


void CPropWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	Invalidate();
}


BOOL CPropWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;//CWnd::OnEraseBkgnd(pDC);
}


void CPropWnd::OnDestroy()
{
	CWnd::OnDestroy();

	// TODO: Add your message handler code here
	DeleteBitmaps();
}


void CPropWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	SetCapture();
	m_nGrabIndex=DetectGrabState(&point,true,m_eGType);
	if(m_nGrabIndex>=0&&m_eGType==eGrabTab)
	{
		bool unchanged=false;
		int ctrlid=m_PropStat.SelectTab(m_nGrabIndex,unchanged);
		if(ctrlid>=0&&!unchanged)
		{
		}
	}
	Invalidate();
	CWnd::OnLButtonDown(nFlags, point);
}


void CPropWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture();
	if(m_eGType==eGrabCloseBtn)
	{
		int next;
		int ctrlid=m_PropStat.DeleteTab(m_nGrabIndex,next);
		if(ctrlid>=0&&next>=0)
		{
		}
	}
	E_PROP_GRAB_TYPE type;
	int idx=DetectGrabState(&point,false,type);
	if(idx>=0&&type==eGrabHoverBtn)
	{
		m_eGType=type;
		m_nGrabIndex=idx;
	}
	else
	{
		m_eGType=eGrabNone;
		m_nGrabIndex=-1;
	}
	Invalidate();
	CWnd::OnLButtonUp(nFlags, point);
}


void CPropWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	TRACKMOUSEEVENT tme;
	ZeroMemory(&tme,sizeof(tme));
	tme.cbSize=sizeof(TRACKMOUSEEVENT);
	tme.dwFlags=TME_LEAVE;
	tme.hwndTrack=m_hWnd;
	TrackMouseEvent(&tme);

	E_PROP_GRAB_TYPE type;
	int idx=DetectGrabState(&point,!!(nFlags&MK_LBUTTON),type);
	bool bUpdate=false;
	switch(m_eGType)
	{
	case eGrabHoverBtn:
		if(type!=eGrabHoverBtn)
		{
			m_eGType=eGrabNone;
			m_nGrabIndex=-1;
			bUpdate=true;
		}
		break;
	case eGrabCloseBtn:
		m_eGType=eGrabNone;
		m_nGrabIndex=-1;
		bUpdate=true;
		break;
	case eGrabTab:
		break;
	default:
		if(type==eGrabHoverBtn)
		{
			m_nGrabIndex=idx;
			m_eGType=type;
			bUpdate=true;
		}
		break;
	}
	if(bUpdate)
		Invalidate();
	CWnd::OnMouseMove(nFlags, point);
}


void CPropWnd::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default
	m_eGType=eGrabNone;
	m_nGrabIndex=-1;
	Invalidate();
	CWnd::OnMouseLeave();
}


void CPropWnd::OnFileNewTab()
{
	// TODO: Add your command handler code here
	m_PropStat.NewTab();
	Invalidate();
}