#pragma once

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

private:
	TreeListTabGrid m_tabLeft;
	TreeListTabGrid m_tabRight;
	TabStat m_tabStat;

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnSizeView(WPARAM wParam,LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
};
