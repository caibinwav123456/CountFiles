#include "pch.h"
#include "HeadBar.h"
#include "common.h"
#include "resource.h"

TabStat::TabStat()
{
	flag=TLTAB_ALL;
	column_names[0].LoadString(IDS_TAB_NAME);
	column_names[1].LoadString(IDS_TAB_SIZE);
	column_names[2].LoadString(IDS_TAB_MODIFY);
}

class TreeListTabSplitter
{
private:
	TreeListTabGrid* m_tab;
public:
	TreeListTabSplitter(TreeListTabGrid* tab):m_tab(tab){}
	void ArrangeTabs(bool col_changed);
	void clear();
	size_t size();
	TabItem& operator[](size_t idx);
	void insert(size_t idx,const TabItem& item);
	void insert(size_t idx,const CRect& rc,const CString& title);
	void push(const TabItem& item);
	void push(const CRect& rc,const CString& title);
	TabItem pop();
	void erase(size_t idx);

	bool get_at(UINT pos,CRect& rc,CString& title);
	bool set_at(UINT pos,const CString& title);
	bool clear_at(UINT pos);
	bool has_tab(UINT pos){return !!(m_tab->mask&pos);}
};
void TreeListTabSplitter::ArrangeTabs(bool col_changed)
{

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
	m_tab->arrTab.insert(m_tab->arrTab.begin(),item);
}
void TreeListTabSplitter::insert(size_t idx,const CRect& rc,const CString& title)
{
	m_tab->arrTab.insert(m_tab->arrTab.begin(),TabItem(rc,title));
}
void TreeListTabSplitter::push(const TabItem& item)
{
	m_tab->arrTab.push_back(item);
}
void TreeListTabSplitter::push(const CRect& rc,const CString& title)
{
	push(TabItem(rc,title));
}
TabItem TreeListTabSplitter::pop()
{
	TabItem back= m_tab->arrTab.back(); m_tab->arrTab.pop_back();return back;
}
void TreeListTabSplitter::erase(size_t idx)
{
	m_tab->arrTab.erase(m_tab->arrTab.begin());
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
				TabItem item(CRect(0,0,0,LINE_HEIGHT),title);
				tab.arrTab.insert(tab.arrTab.begin()+idx,item);
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
			tab.arrTab.erase(tab.arrTab.begin()+idx);
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
END_MESSAGE_MAP()

CHeadBar::CHeadBar()
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
	splitter.ArrangeTabs(colchg);
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

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}

LRESULT CHeadBar::OnSizeView(WPARAM wParam,LPARAM lParam)
{
	CRect rc=*(CRect*)wParam;
	rc.bottom=LINE_HEIGHT;
	SplitTab(rc);
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
