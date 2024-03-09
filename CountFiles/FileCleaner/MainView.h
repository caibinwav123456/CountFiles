
// MainView.h : interface of the CMainView class
//


#pragma once

#include "TreeListCtrl.h"
// CMainView window
class CScrollTreeList:public TreeListCtrl
{
public:
	CScrollTreeList(CWnd* pWnd);
public:
	virtual void SetScrollPos(const CPoint& pos);
	virtual CPoint GetScrollPos() const;
	virtual void SetScrollSizes(const CSize& size);
	virtual CSize GetScrollSizes() const;
private:
	CSize m_sizeScl;
};
class CMainView : public CScrollView
{
	DECLARE_DYNCREATE(CMainView)

// Construction
public:
	CMainView();

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
	virtual ~CMainView();

// Generated message map functions
protected:
	virtual void OnDraw(CDC* pDC);

	DECLARE_ID2WND_MAP(CMainView)

public:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnStartLoadList(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRearrangeTabSize(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnExportListFile(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnExportIsValid(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewSession(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseSession(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSwitchSession(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};