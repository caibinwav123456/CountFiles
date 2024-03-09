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
inline void update_path_title(const string& path,CString& fullpath,CString& title)
{
	int pos=(int)path.rfind("\\");
	string strtitle=pos==string::npos?path:path.substr(pos+1);
	fullpath=a2tstr(path),title=a2tstr(strtitle);
}
void PropTabData::UpdateString(const string& left,const string& right)
{
	update_path_title(left,lPath,lTitle);
	update_path_title(right,rPath,rTitle);
}
PropTabData* PropTabStat::GetSelTab()
{
	return first.next==&last?NULL:first.next;
}
void PropTabStat::NewTab(const string& left,const string& right)
{
	PropTabData* data=new PropTabData;
	PropTabData* cursel=GetSelTab();
	data->UpdateString(left,right);
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
void PropTabStat::SetCurTabString(const string& left,const string& right)
{
	PropTabData* cursel=GetSelTab();
	if(cursel!=NULL)
		cursel->UpdateString(left,right);
}
CPropWnd::CPropWnd():m_iBaseX(PROP_START_X),m_iShiftTab(0),m_nGrabIndex(-1),m_bShowMove(FALSE),m_eGType(eGrabNone)
{

}
CPropWnd::~CPropWnd()
{

}
int CPropWnd::GetPropCount()
{
	return (int)m_PropStat.vecData.size();
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
		{&m_bmpTabLeftN,IDB_BMP_TABL_N},
		{&m_bmpTabLeftH,IDB_BMP_TABL_H},
		{&m_bmpTabLeftC,IDB_BMP_TABL_C},
		{&m_bmpTabLeftD,IDB_BMP_TABL_D},
		{&m_bmpTabRightN,IDB_BMP_TABR_N},
		{&m_bmpTabRightH,IDB_BMP_TABR_H},
		{&m_bmpTabRightC,IDB_BMP_TABR_C},
		{&m_bmpTabRightD,IDB_BMP_TABR_D},
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
	if((m_eGType==eGrabHoverBtn||m_eGType==eGrabCloseBtn)
		&&m_nGrabIndex>=0&&m_PropStat.vecData[m_nGrabIndex]==tab)
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
	DrawTab(drawer,xpos,tab->tab_idx,tab->lTitle,tab->rTitle,tab->issel,state);
}
void CPropWnd::GetMoveBtnRect(RECT* rcl,RECT* rcr)
{
	if(!m_bShowMove)
	{
		*rcl=CRect(0,0,0,0);
		*rcr=CRect(0,0,0,0);
		return;
	}
	CRect rect;
	GetClientRect(&rect);
	*rcl=PROP_TABSL_RECT,*rcr=PROP_TABSR_RECT;
	((CRect*)rcl)->OffsetRect(rect.right,rect.top);
	((CRect*)rcr)->OffsetRect(rect.right,rect.top);
}
void CPropWnd::GetMoveBtnEnableState(bool& ldisable,bool& rdisable)
{
	CRect rect;
	GetClientRect(&rect);
	ldisable=(m_iShiftTab==0);
	rdisable=((int)m_PropStat.vecData.size()<=1||
		(((int)m_PropStat.vecData.size()-m_iShiftTab)*PAGE_TAB_WIDTH<=rect.Width()));
}
void CPropWnd::DrawMoveBtn(CDrawer& drawer)
{
	if(!m_bShowMove)
		return;
	CRect rcLeft,rcRight;
	bool bLeftEnd,bRightEnd;
	GetMoveBtnRect(rcLeft,rcRight);
	GetMoveBtnEnableState(bLeftEnd,bRightEnd);
	CBitmap* bmpLeft=bLeftEnd?&m_bmpTabLeftD:&m_bmpTabLeftN;
	CBitmap* bmpRight=bRightEnd?&m_bmpTabRightD:&m_bmpTabRightN;
	switch(m_eGType)
	{
	case eGrabHoverMove:
		if(m_nGrabIndex==0&&!bLeftEnd)
			bmpLeft=&m_bmpTabLeftH;
		else if(m_nGrabIndex==1&&!bRightEnd)
			bmpRight=&m_bmpTabRightH;
		break;
	case eGrabMove:
		if(m_nGrabIndex==0&&!bLeftEnd)
			bmpLeft=&m_bmpTabLeftC;
		else if(m_nGrabIndex==1&&!bRightEnd)
			bmpRight=&m_bmpTabRightC;
		break;
	}
	CRect rcLMask=rcLeft,rcRMask=rcRight;
	rcLMask.InflateRect(0,0,1,1);
	rcRMask.InflateRect(0,0,1,1);
	drawer.FillRect(rcLMask,RGB(0,0,0));
	drawer.FillRect(rcRMask,RGB(0,0,0));
	drawer.DrawBitmapScaled(bmpLeft,rcLeft,NULL,SRCPAINT);
	drawer.DrawBitmapScaled(bmpRight,rcRight,NULL,SRCPAINT);
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

	CRect rcBtnMask=rcBtn;
	rcBtnMask.InflateRect(0,0,1,1);
	drawer.FillRect(rcBtnMask,RGB(0,0,0));
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
	int rwidth=width;
	if(width>szLeft.cx)
	{
		width=szLeft.cx;
		rwidth=rcString.Width()-sz.cx-width;
	}
	CRect rcLeft(rcString.TopLeft(),CPoint(rcString.left+width,rcString.bottom)),
		rcRight(CPoint(rcString.right-rwidth,rcString.top),rcString.BottomRight());
	CRect rcMid(rcLeft.right,rcLeft.top,rcRight.left,rcRight.bottom);
	drawer.DrawText(rcLeft,DT_ALIGN_LEFT,left,TEXT_HEIGHT,RGB(0,0,0),TRANSPARENT,VIEW_FONT);
	drawer.DrawText(rcMid,DT_ALIGN_LEFT,sep,TEXT_HEIGHT,RGB(0,0,0),TRANSPARENT,VIEW_FONT);
	drawer.DrawText(rcRight,DT_ALIGN_LEFT,right,TEXT_HEIGHT,RGB(0,0,0),TRANSPARENT,VIEW_FONT);
}
int CPropWnd::DetectMoveBtnState(LPPOINT pt,bool mousedown,E_PROP_GRAB_TYPE& type)
{
	if(!m_bShowMove)
	{
		type=eGrabNone;
		return -1;
	}
	CRect rcBtn[2];
	GetMoveBtnRect(rcBtn[0],rcBtn[1]);
	for(int i=0;i<2;i++)
	{
		if(rcBtn[i].PtInRect(*pt))
		{
			type=(mousedown?eGrabMove:eGrabHoverMove);
			return i;
		}
	}
	type=eGrabNone;
	return -1;
}
int CPropWnd::DetectGrabState(LPPOINT pt,bool mousedown,E_PROP_GRAB_TYPE& type)
{
	E_PROP_GRAB_TYPE typeBtnMove;
	int idxBtnMove=DetectMoveBtnState(pt,mousedown,typeBtnMove);
	if(typeBtnMove!=eGrabNone)
	{
		type=typeBtnMove;
		return idxBtnMove;
	}
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
	CPoint relative_coord(x-idx*PAGE_TAB_WIDTH,point.y);
	if(PROP_BUTTON_RECT.PtInRect(relative_coord))
		type=(mousedown?eGrabCloseBtn:eGrabHoverBtn);
	else
		type=eGrabTab;
	return idx;
}
void CPropWnd::AdjustMoveBtn(int width)
{
	if(width<=0)
	{
		CRect rect;
		GetClientRect(&rect);
		width=rect.Width();
	}
	for(;m_iShiftTab>0;m_iShiftTab--)
	{
		if(width<((int)m_PropStat.vecData.size()+1-m_iShiftTab)*PAGE_TAB_WIDTH)
			break;
	}
	m_bShowMove=(((int)m_PropStat.vecData.size())*PAGE_TAB_WIDTH>width);
	m_iBaseX=PROP_START_X-m_iShiftTab*PAGE_TAB_WIDTH;
}
void CPropWnd::AlignNewTab()
{
	CRect rect;
	GetClientRect(&rect);
	int width=rect.Width();
	for(;;m_iShiftTab++)
	{
		if(width>=((int)m_PropStat.vecData.size()-m_iShiftTab)*PAGE_TAB_WIDTH)
			break;
	}
	m_bShowMove=(((int)m_PropStat.vecData.size())*PAGE_TAB_WIDTH>width);
	m_iBaseX=PROP_START_X-m_iShiftTab*PAGE_TAB_WIDTH;
}

BEGIN_MESSAGE_MAP(CPropWnd, CWnd)
	ON_MESSAGE(WM_SIZEPARENT, &CPropWnd::OnSizeParent)
	ON_MESSAGE(WM_SET_PROP_WND_TITLE, &CPropWnd::OnSetCurTitle)
	ON_MESSAGE(WM_CLOSE_CURRENT_SESSION, &CPropWnd::OnCloseCurrentSession)
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
	DrawMoveBtn(drawer);
}


void CPropWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	AdjustMoveBtn(cx);
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
	if(m_eGType==eGrabTab&&m_nGrabIndex>=0)
	{
		bool unchanged=false;
		int ctrlid=m_PropStat.SelectTab(m_nGrabIndex,unchanged);
		if(ctrlid>=0&&!unchanged)
		{
			SendMessageToIDWnd(IDW_MAIN_VIEW,WM_SWITCH_SESSION,(WPARAM)ctrlid);
		}
	}
	Invalidate();
	CWnd::OnLButtonDown(nFlags, point);
}


void CPropWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture();
	SetCursor(LoadCursor(NULL,IDC_ARROW));
	switch(m_eGType)
	{
	case eGrabCloseBtn:
		{
			int next;
			int ctrlid=m_PropStat.DeleteTab(m_nGrabIndex,next);
			AdjustMoveBtn();
			if(ctrlid>=0)
			{
				SendMessageToIDWnd(IDW_MAIN_VIEW,WM_CLOSE_SESSION,(WPARAM)ctrlid,(LPARAM)next);
			}
		}
		break;
	case eGrabMove:
		{
			bool bLEnd,bREnd;
			GetMoveBtnEnableState(bLEnd,bREnd);
			if(m_nGrabIndex==0&&!bLEnd)
			{
				m_iShiftTab--;
				ASSERT(m_iShiftTab>=0);
				m_iBaseX=PROP_START_X-m_iShiftTab*PAGE_TAB_WIDTH;
			}
			else if(m_nGrabIndex==1&&!bREnd)
			{
				m_iShiftTab++;
				m_iBaseX=PROP_START_X-m_iShiftTab*PAGE_TAB_WIDTH;
			}
		}
		break;
	}
	E_PROP_GRAB_TYPE type;
	int idx=DetectGrabState(&point,false,type);
	switch(type)
	{
	case eGrabTab:
		if(idx!=m_nGrabIndex)
			m_PropStat.ReorderTab(m_nGrabIndex,idx);
		m_eGType=eGrabNone;
		m_nGrabIndex=-1;
		break;
	case eGrabHoverBtn:
	case eGrabHoverMove:
		m_eGType=type;
		m_nGrabIndex=idx;
		break;
	default:
		m_eGType=eGrabNone;
		m_nGrabIndex=-1;
		break;
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
		if(type==eGrabTab&&idx>=0&&m_nGrabIndex>=0&&idx!=m_nGrabIndex)
			SetCursor(LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_CUR_MOVE)));
		else
			SetCursor(LoadCursor(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDC_CUR_BAN)));
		break;
	case eGrabMove:
	case eGrabHoverMove:
		if(type!=m_eGType||idx!=m_nGrabIndex)
		{
			if(type!=eGrabTab&&type!=eGrabCloseBtn)
			{
				m_eGType=type;
				m_nGrabIndex=idx;
			}
			else
			{
				m_eGType=eGrabNone;
				m_nGrabIndex=-1;
			}
			bUpdate=true;
		}
		break;
	default:
		if(type==eGrabHoverBtn||type==eGrabHoverMove)
		{
			m_eGType=type;
			m_nGrabIndex=idx;
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
	SetCursor(LoadCursor(NULL,IDC_ARROW));
	Invalidate();
	CWnd::OnMouseLeave();
}


void CPropWnd::OnFileNewTab()
{
	// TODO: Add your command handler code here
	m_PropStat.NewTab();
	AlignNewTab();
	SendMessageToIDWnd(IDW_MAIN_VIEW,WM_NEW_SESSION);
	Invalidate();
}

LRESULT CPropWnd::OnSetCurTitle(WPARAM wParam, LPARAM lParam)
{
	const string& left=*(string*)wParam;
	const string& right=*(string*)lParam;
	m_PropStat.SetCurTabString(left,right);
	Invalidate();
	return 0;
}

LRESULT CPropWnd::OnCloseCurrentSession(WPARAM wParam, LPARAM lParam)
{
	int idx;
	PropTabData* pProp=m_PropStat.GetSelTab();
	if(pProp==NULL)
		return 0;
	for(idx=0;idx<(int)m_PropStat.vecData.size();idx++)
	{
		if(m_PropStat.vecData[idx]==pProp)
			break;
	}
	if(idx>=(int)m_PropStat.vecData.size())
		return 0;
	int next;
	int ctrlid=m_PropStat.DeleteTab(idx,next);
	AdjustMoveBtn();
	if(ctrlid>=0)
	{
		SendMessageToIDWnd(IDW_MAIN_VIEW,WM_CLOSE_SESSION,(WPARAM)ctrlid,(LPARAM)next);
	}
	Invalidate();
	return 0;
}