
// ChildView.h : interface of the CChildView class
//


#pragma once

#include "TreeListCtrl.h"
// CChildView window
class CScrollTreeList:public TreeListCtrl
{
public:
	CScrollTreeList(CWnd* pWnd);
protected:
	virtual CPoint GetScrollPos() const;
	virtual void SetScrollSizes(const CSize& size);
};
class CChildView : public CScrollView
{
	DECLARE_DYNCREATE(CChildView)

// Construction
public:
	CChildView();

// Attributes
public:
	CScrollTreeList m_TreeList;

// Operations
public:

// Overrides
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

// Generated message map functions
protected:
	virtual void OnDraw(CDC* pDC);
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
};

