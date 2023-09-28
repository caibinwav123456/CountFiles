#pragma once
#include "DrawObject.h"

struct TabStat
{
	UINT flag;
	CString column_names[3];
	int default_width[3];
	int min_width[3];
	TabStat();
};

class CHeadBar :
	public CWnd
{
	DECLARE_DYNAMIC(CHeadBar)
	DECLARE_ID2WND_MAP(CHeadBar)
public:
	CHeadBar();
	~CHeadBar();

private:
	void SplitTab(CRect& rect,UINT flag);
	void SplitTab(CRect& rect);
	void CalcTabStat(TreeListTabGrid& tab,CRect& rc,UINT flag);
	void DrawTabs(CDrawer* pDrawer,const TreeListTabGrid& tab);

	TreeListTabGrid* DetectGrabStatus(CPoint pt,int& index);
	void RepositionTab(int xpos);

private:
	TreeListTabGrid m_tabLeft;
	TreeListTabGrid m_tabRight;
	TabStat m_tabStat;
	CRect m_rectBar;
	int m_iOrgX;

	int m_GrabIndex;
	TreeListTabGrid* m_pTabGrabbed;

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();

	virtual void OnDraw(CDC* pDC);

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnSizeView(WPARAM wParam,LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};
