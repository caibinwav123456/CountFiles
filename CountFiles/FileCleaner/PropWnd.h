#ifndef _PROP_WND_H_
#define _PROP_WND_H_
#include "DrawObject.h"
class CPropWnd :
	public CWnd
{
	DECLARE_DYNAMIC(CPropWnd)
	DECLARE_ID2WND_MAP(CPropWnd)
public:
	CPropWnd();
	~CPropWnd();
private:
	CBitmap m_bmpBk;

private:
	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
};
#endif