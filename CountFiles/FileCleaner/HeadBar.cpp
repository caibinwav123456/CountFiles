#include "pch.h"
#include "HeadBar.h"
#include "common.h"
#include "resource.h"
#define clamp_value(val,minimum,maximum) \
	if(val<minimum) \
		val=minimum; \
	else if(val>maximum) \
		val=maximum;

TabStat::TabStat()
{
	flag=0;

	column_names[0].LoadString(IDS_TAB_NAME);
	column_names[1].LoadString(IDS_TAB_SIZE);
	column_names[2].LoadString(IDS_TAB_MODIFY);

	default_width[0]=DEFAULT_TABWIDTH_NAME;
	default_width[1]=DEFAULT_TABWIDTH_SIZE;
	default_width[2]=DEFAULT_TABWIDTH_TIME;

	min_width[0]=MIN_TABWIDTH_NAME;
	min_width[1]=MIN_TABWIDTH_OTHER;
	min_width[2]=MIN_TABWIDTH_OTHER;
}
inline int GetTabMaskIndex(const TreeListTabGrid& tab,int index)
{
	for(int i=0,idx=0;i<32;i++)
	{
		UINT flags=(1<<i);
		if(tab.mask&flags)
		{
			if((idx++)==index)
				return i;
		}
	}
	return -1;
}

inline bool InGrabRegion(CPoint pt,int org)
{
	return pt.x>=org-TAB_GRAB_BIAS&&pt.x<=org+TAB_GRAB_BIAS;
}
inline int GetGrabIndex(CPoint pt,const TreeListTabGrid& tab)
{
	for(int i=0;i<(int)tab.arrTab.size()-1;i++)
	{
		if(InGrabRegion(pt,tab.arrTab[i].rect.right))
			return i;
	}
	return -1;
}

class TreeListTabSplitter
{
private:
	TreeListTabGrid* m_tab;
public:
	TreeListTabSplitter(TreeListTabGrid* tab):m_tab(tab){}
	void ArrangeTabs(bool col_changed,TabStat* pstat);
	void clear();
	size_t size();
	TabItem& operator[](size_t idx);
	void insert(size_t idx,const TabItem& item);
	void insert(size_t idx,const CRect& rc,const CString& title);
	void push(const TabItem& item);
	void push(const CRect& rc,const CString& title);
	void pop();
	TabItem& back();
	void erase(size_t idx);

	bool get_at(UINT pos,CRect& rc,CString& title);
	bool set_at(UINT pos,const CString& title);
	bool clear_at(UINT pos);
	bool has_tab(UINT pos){return !!(m_tab->mask&pos);}
};
void TreeListTabSplitter::ArrangeTabs(bool col_changed,TabStat* pstat)
{
	ASSERT(has_tab(TLTAB_NAME));
	TreeListTabGrid& tab=*m_tab;
	vector<int> idx_cache;
	int r=tab.rcTotal.right,
		l=tab.rcTotal.left;
	for(int i=0;i<32;i++)
	{
		UINT flags=(1<<i);
		if(tab.mask&flags)
			idx_cache.push_back(i);
	}
	if((int)size()==0)
		return;
	if(col_changed)
	{
		for(int i=(int)size()-1;i>=0;i--)
		{
			TabItem& item=tab.arrTab[i];
			item.rect.right=r;
			r-=pstat->default_width[idx_cache[i]];
			item.rect.left=r;
		}
		if(r-l>=0)
			tab.arrTab[0].rect.left=l;
		else
		{
			int insufficiency=0;
			for(int i=0;i<(int)size();i++)
			{
				insufficiency+=pstat->default_width[idx_cache[i]]
					-pstat->min_width[idx_cache[i]];
			}
			int insuf=insufficiency+r-l;
			if(insuf>0)
			{
				insuf=l-r;
				for(int i=0;i<(int)size();i++)
				{
					int discount=pstat->default_width[idx_cache[i]]
						-pstat->min_width[idx_cache[i]];
					if(discount>insuf)
						discount=insuf;
					insuf-=discount;
					tab.arrTab[i].rect.left=l;
					l+=pstat->default_width[idx_cache[i]]-discount;
					tab.arrTab[i].rect.right=l;
				}
			}
			else
			{
				insuf=-insuf;
				tab.arrTab[0].rect.left=l;
				r=tab.arrTab[0].rect.right=l+pstat->min_width[0];
				for(int i=1;i<(int)size();i++)
				{
					int discount=pstat->min_width[idx_cache[i]];
					if(discount>insuf)
						discount=insuf;
					insuf-=discount;
					tab.arrTab[i].rect.left=r;
					r+=pstat->min_width[idx_cache[i]]-discount;
					tab.arrTab[i].rect.right=r;
				}
			}
		}
	}
	else
	{
		int movoffset=l-tab.arrTab[0].rect.left;
		for(int i=0;i<(int)size();i++)
			tab.arrTab[i].rect.OffsetRect(movoffset,0);
		int diffw=r-back().rect.right;
		UINT state=0;
		int index=-1;
		if(diffw==0)
			return;
		int sufficient_size=0;
		for(int i=0;i<(int)size();i++)
			sufficient_size+=pstat->default_width[idx_cache[i]];
		if(tab.rcTotal.Width()>=sufficient_size)
		{
			for(int i=(int)size()-1;i>=0;i--)
			{
				tab.arrTab[i].rect.right=r;
				r-=pstat->default_width[idx_cache[i]];
				tab.arrTab[i].rect.left=r;
			}
			tab.arrTab[0].rect.left=l;
			ASSERT(tab.arrTab[0].rect.left==tab.rcTotal.left
				&&back().rect.right==tab.rcTotal.right);
			return;
		}
		if(diffw>0)
		{
#define state_has_below_def 1
#define state_has_below_min 2
			for(int i=0;i<(int)size();i++)
			{
				if(tab.arrTab[i].rect.Width()
					<pstat->min_width[idx_cache[i]])
				{
					switch(state)
					{
					case 0:
					case state_has_below_def:
						state=state_has_below_min;
						index=i;
						break;
					case state_has_below_min:
						if(tab.arrTab[i].rect.Width()
							<tab.arrTab[index].rect.Width())
							index=i;
						break;
					}
				}
				else if(tab.arrTab[i].rect.Width()
					<pstat->default_width[idx_cache[i]])
				{
					switch(state)
					{
					case 0:
						state=state_has_below_def;
						index=i;
						break;
					case state_has_below_def:
						if(tab.arrTab[i].rect.Width()
							<tab.arrTab[index].rect.Width())
							index=i;
						break;
					case state_has_below_min:
						break;
					}
				}
			}
			switch(state)
			{
			case 0:
				tab.arrTab[0].rect.right+=diffw;
				for(int i=1;i<(int)size();i++)
					tab.arrTab[i].rect.OffsetRect(diffw,0);
				break;
			case state_has_below_def:
			case state_has_below_min:
				tab.arrTab[index].rect.right+=diffw;
				for(int i=index+1;i<(int)size();i++)
					tab.arrTab[i].rect.OffsetRect(diffw,0);
				break;
			}
		}
		else
		{
#define state_has_above_min 1
#define state_has_above_def 2
			for(int i=0;i<(int)size();i++)
			{
				if(tab.arrTab[i].rect.Width()
					>pstat->default_width[idx_cache[i]])
				{
					switch(state)
					{
					case 0:
					case state_has_above_min:
						state=state_has_above_def;
						index=i;
						break;
					case state_has_above_def:
						if(tab.arrTab[i].rect.Width()
							>tab.arrTab[index].rect.Width())
							index=i;
						break;
					}
				}
				else if(tab.arrTab[i].rect.Width()
					>pstat->min_width[idx_cache[i]])
				{
					switch(state)
					{
					case 0:
						state=state_has_above_min;
						index=i;
						break;
					case state_has_above_min:
						if(tab.arrTab[i].rect.Width()
							>tab.arrTab[index].rect.Width())
							index=i;
						break;
					case state_has_above_def:
						break;
					}
				}
			}
			int index_consume;
			switch(state)
			{
			case 0:
				diffw=-diffw-(tab.arrTab[0].rect.Width()-pstat->min_width[0]);
				l=tab.arrTab[0].rect.right=l+pstat->min_width[0];
				for(int i=1;i<(int)size();i++)
				{
					int width=tab.arrTab[i].rect.Width();
					int consume=width;
					tab.arrTab[i].rect.left=l;
					if(consume>diffw)
						consume=diffw;
					diffw-=consume;
					l+=(width-consume);
					tab.arrTab[i].rect.right=l;
				}
				break;
			case state_has_above_min:
			case state_has_above_def:
				index_consume=(index==0?tab.arrTab[0].rect.Width()-pstat->min_width[0]+diffw
					:tab.arrTab[index].rect.Width()+diffw);
				if(index_consume>=0)
				{
					tab.arrTab[index].rect.right+=diffw;
					for(int i=index+1;i<(int)size();i++)
						tab.arrTab[i].rect.OffsetRect(diffw,0);
				}
				else
				{
					index_consume=-index_consume;
					for(int i=0;i<(int)size();i++)
					{
						int width=tab.arrTab[i].rect.Width();
						int consume=width;
						tab.arrTab[i].rect.left=l;
						if(i==index)
						{
							if(i==0)
								l+=pstat->min_width[0];
						}
						else
						{
							if(consume>index_consume)
								consume=index_consume;
							index_consume-=consume;
							l+=(width-consume);
						}
						tab.arrTab[i].rect.right=l;
					}
				}
				break;
			}
		}
	}
	ASSERT(tab.arrTab[0].rect.left==tab.rcTotal.left
		&&back().rect.right==tab.rcTotal.right);
}
void TreeListTabSplitter::clear()
{
	m_tab->mask=0;
	m_tab->arrTab.clear();
}
size_t TreeListTabSplitter::size()
{
	return m_tab->arrTab.size();
}
TabItem& TreeListTabSplitter::operator[](size_t idx)
{
	return m_tab->arrTab[idx];
}
void TreeListTabSplitter::insert(size_t idx,const TabItem& item)
{
	m_tab->arrTab.insert(m_tab->arrTab.begin()+idx,item);
}
void TreeListTabSplitter::insert(size_t idx,const CRect& rc,const CString& title)
{
	insert(idx,TabItem(rc,title));
}
void TreeListTabSplitter::push(const TabItem& item)
{
	m_tab->arrTab.push_back(item);
}
void TreeListTabSplitter::push(const CRect& rc,const CString& title)
{
	push(TabItem(rc,title));
}
void TreeListTabSplitter::pop()
{
	m_tab->arrTab.pop_back();
}
TabItem& TreeListTabSplitter::back()
{
	return m_tab->arrTab.back();
}
void TreeListTabSplitter::erase(size_t idx)
{
	m_tab->arrTab.erase(m_tab->arrTab.begin()+idx);
}

bool TreeListTabSplitter::get_at(UINT pos,CRect& rc,CString& title)
{
	TreeListTabGrid& tab=*m_tab;
	for(int i=0,idx=0;i<32;i++)
	{
		UINT flag=(1<<i);
		size_t index;
		if(tab.mask&flag)
			index=(idx++);
		else
			continue;
		if(pos==flag)
		{
			rc=tab.arrTab[index].rect;
			title=tab.arrTab[index].title;
			return true;
		}
	}
	return false;
}
bool TreeListTabSplitter::set_at(UINT pos,const CString& title)
{
	TreeListTabGrid& tab=*m_tab;
	for(int i=0,idx=0;i<32;i++)
	{
		UINT flag=(1<<i);
		if(pos==flag)
		{
			if(tab.mask&flag)
			{
				tab.arrTab[idx].title=title;
			}
			else
			{
				tab.mask|=flag;
				insert(idx,CRect(0,0,0,LINE_HEIGHT),title);
			}
			return true;
		}
		if(tab.mask&flag)
			idx++;
	}
	return false;
}
bool TreeListTabSplitter::clear_at(UINT pos)
{
	TreeListTabGrid& tab=*m_tab;
	if(!(tab.mask&pos))
		return false;
	for(int i=0,idx=0;i<32;i++)
	{
		UINT flag=(1<<i);
		if(pos==flag&&(tab.mask&flag))
		{
			tab.mask&=(~flag);
			erase(idx);
			return true;
		}
		if(tab.mask&flag)
			idx++;
	}
	return false;
}

IMPLEMENT_DYNAMIC(CHeadBar,CWnd)
IMPLEMENT_ID2WND_MAP(CHeadBar,IDW_HEAD_BAR)

BEGIN_MESSAGE_MAP(CHeadBar,CWnd)
	ON_MESSAGE(WM_SET_VIEW_SIZE,&CHeadBar::OnSizeView)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

CHeadBar::CHeadBar():m_iOrgX(0),m_GrabIndex(-1),m_pTabGrabbed(NULL)
{

}
CHeadBar::~CHeadBar()
{

}

void CHeadBar::CalcTabStat(TreeListTabGrid& tab,CRect& rc,UINT newflag)
{
	tab.rcTotal=rc;
	TreeListTabSplitter splitter(&tab);
	bool colchg=false;
	for(int i=0;i<32;i++)
	{
		UINT flag=(1<<i);
		if(!(flag&TLTAB_ALL))
			continue;
		if(splitter.has_tab(flag)&&!(flag&newflag))
		{
			splitter.clear_at(flag);
			colchg=true;
		}
		else if((!splitter.has_tab(flag))&&(flag&newflag))
		{
			splitter.set_at(flag,m_tabStat.column_names[i]);
			colchg=true;
		}
	}
	splitter.ArrangeTabs(colchg,&m_tabStat);
}

void CHeadBar::SplitTab(CRect& rect,UINT flag)
{
	int grpwidth=(max(MIN_SCROLL_WIDTH,rect.Width())-BAR_CENTER_SPACE)/2;
	CRect rcl(0,0,grpwidth,rect.bottom),
		rcr(grpwidth+BAR_CENTER_SPACE,0,grpwidth*2+BAR_CENTER_SPACE,rect.bottom);
	CalcTabStat(m_tabLeft,rcl,flag);
	CalcTabStat(m_tabRight,rcr,flag);
	m_tabStat.flag=flag;
}
void CHeadBar::SplitTab(CRect& rect)
{
	SplitTab(rect,m_tabStat.flag);
}

BOOL CHeadBar::PreCreateWindow(CREATESTRUCT& cs)
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

TreeListTabGrid* CHeadBar::DetectGrabStatus(CPoint pt,int& index)
{
	if((index=GetGrabIndex(pt,m_tabLeft))>=0)
		return &m_tabLeft;
	else if((index=GetGrabIndex(pt,m_tabRight))>=0)
		return &m_tabRight;
	index=-1;
	return NULL;
}

void CHeadBar::RepositionTab(int xpos)
{
	ASSERT(m_pTabGrabbed!=NULL&&m_GrabIndex>=0);
	ASSERT((m_pTabGrabbed->mask&TLTAB_NAME)&&!m_pTabGrabbed->arrTab.empty());
	ASSERT(m_pTabGrabbed->arrTab[0].rect.Width()>=m_tabStat.min_width[0]);
	int maskoff=GetTabMaskIndex(*m_pTabGrabbed,m_GrabIndex);
	if((int)m_pTabGrabbed->arrTab.size()==1)
		return;
	int left=m_pTabGrabbed->arrTab[m_GrabIndex].rect.left;
	if(m_GrabIndex==0)
		left+=m_tabStat.min_width[0];
	int right=m_pTabGrabbed->arrTab[m_GrabIndex+1].rect.right;
	clamp_value(xpos,left,right)
	m_pTabGrabbed->arrTab[m_GrabIndex].rect.right=
		m_pTabGrabbed->arrTab[m_GrabIndex+1].rect.left=xpos;
}

void CHeadBar::DrawTabs(CDrawer* pDrawer,const TreeListTabGrid& tab)
{
	for(int i=0;i<(int)tab.arrTab.size();i++)
	{
		if(i!=0)
			pDrawer->DrawLine(&CPoint(tab.arrTab[i].rect.left,0),&CPoint(tab.arrTab[i].rect.left,LINE_HEIGHT),
				TAB_SEP_COLOR,3,PS_SOLID);
		CRect rc=tab.arrTab[i].rect;
		rc.left+=TAB_GRAB_BIAS;
		pDrawer->DrawText(&rc,DT_ALIGN_LEFT,(LPCTSTR)tab.arrTab[i].title,LINE_HEIGHT,RGB(0,0,0),TRANSPARENT,VIEW_FONT);
	}
}

void CHeadBar::OnDraw(CDC* pDC)
{
	CDCDraw canvas(this,pDC,true);
	CDrawer drawer(&canvas);
	CRect rect;
	GetClientRect(&rect);
	ASSERT(rect.left==0&&rect.top==0);
	if(rect.right<MIN_SCROLL_WIDTH)
		rect.right=MIN_SCROLL_WIDTH;

	rect.InflateRect(1,1,1,1);
	drawer.FillRect(&rect,RGB(255,255,255));
	DrawTabs(&drawer,m_tabLeft);
	DrawTabs(&drawer,m_tabRight);

	drawer.DrawLine(&CPoint(m_tabLeft.rcTotal.right,0),&m_tabLeft.rcTotal.BottomRight(),
		BACK_GREY_COLOR,1,PS_SOLID);
	drawer.DrawLine(&m_tabRight.rcTotal.TopLeft(),&CPoint(m_tabRight.rcTotal.left,LINE_HEIGHT),
		BACK_GREY_COLOR,1,PS_SOLID);
}

void CHeadBar::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	delete this;
}

LRESULT CHeadBar::OnSizeView(WPARAM wParam,LPARAM lParam)
{
	m_rectBar=*(CRect*)wParam;
	m_iOrgX=m_rectBar.left;
	m_rectBar.bottom=LINE_HEIGHT;
	m_rectBar.left=0;
	SplitTab(m_rectBar);
	Invalidate();
	TabInfo tab(&m_tabLeft,&m_tabRight);
	return SendMessageToIDWnd(IDW_MAIN_VIEW,WM_REARRANGE_TAB_SIZE,(WPARAM)&tab);
}

int CHeadBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CRect rc(0,0,MIN_SCROLL_WIDTH,LINE_HEIGHT);
	SplitTab(rc,TLTAB_INITIAL);

	return 0;
}


BOOL CHeadBar::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;//CWnd::OnEraseBkgnd(pDC);
}

void CHeadBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CWnd::OnPaint() for painting messages
	dc.SetViewportOrg(CPoint(-m_iOrgX,0));
	OnDraw(&dc);
}

void CHeadBar::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if(m_pTabGrabbed==NULL)
	{
		int index=-1;
		TreeListTabGrid* pTabGrabbed=DetectGrabStatus(point,index);
		if(pTabGrabbed!=NULL)
			SetCursor(LoadCursor(NULL,IDC_SIZEWE));
		else
			SetCursor(LoadCursor(NULL,IDC_ARROW));
	}
	else
	{
		SetCursor(LoadCursor(NULL,IDC_SIZEWE));
		RepositionTab(point.x);
		Invalidate();
		TabInfo tab(&m_tabLeft,&m_tabRight);
		SendMessageToIDWnd(IDW_MAIN_VIEW,WM_REARRANGE_TAB_SIZE,(WPARAM)&tab);
	}
	CWnd::OnMouseMove(nFlags, point);
}


void CHeadBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_pTabGrabbed=DetectGrabStatus(point,m_GrabIndex);
	if(m_pTabGrabbed!=NULL)
	{
		SetCursor(LoadCursor(NULL,IDC_SIZEWE));
		SetCapture();
	}
	else
	{
		SetCursor(LoadCursor(NULL,IDC_ARROW));
	}
	CWnd::OnLButtonDown(nFlags, point);
}


void CHeadBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_pTabGrabbed=NULL;
	m_GrabIndex=-1;
	ReleaseCapture();
	SetCursor(LoadCursor(NULL,IDC_ARROW));
	CWnd::OnLButtonUp(nFlags, point);
}
