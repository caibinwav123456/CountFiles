#ifndef _PROP_WND_H_
#define _PROP_WND_H_
#include "DrawObject.h"
#include <vector>
struct BitmapLoadInfo
{
	CBitmap* bmp;
	int nIDResource;
};
struct PropTabData
{
	int ctrl_idx;
	bool issel;
	CString left;
	CString right;
	PropTabData* prev;
	PropTabData* next;
	PropTabData():ctrl_idx(-1),issel(false),prev(this),next(this){}
	void LinkAfter(PropTabData* data);
	void Remove();
};
struct PropTabStat
{
	vector<PropTabData*> vecData;
	PropTabData first;
	PropTabData last;
	PropTabStat()
	{
		first.prev=NULL;
		first.next=&last;
		last.prev=&first;
		last.next=NULL;
	}
	~PropTabStat()
	{
		for(int i=0;i<(int)vecData.size();i++)
			delete vecData[i];
	}
	PropTabData* GetSelTab();
	void NewTab();
	int DeleteTab(int idx,int& next);
	bool ReorderTab(int origin, int insert_before);
	int SelectTab(int idx,bool& unchanged);
	void SetCurTabString(const CString& left,const CString& right=_T(""));
};
enum E_PROP_BTN_STATE
{
	ePropBtnNormal,
	ePropBtnHigh,
	ePropBtnClick,
	ePropBtnDisable,
};
enum E_PROP_GRAB_TYPE
{
	eGrabNone,
	eGrabTab,
	eGrabCloseBtn,
	eGrabHoverBtn,
};
class CPropWnd:public CWnd
{
	DECLARE_DYNAMIC(CPropWnd)
	DECLARE_ID2WND_MAP(CPropWnd)
public:
	CPropWnd();
	~CPropWnd();

private:
	CBitmap m_bmpBk;
	CBitmap m_bmpTab;
	CBitmap m_bmpTabMask;
	CBitmap m_bmpTabSel;
	CBitmap m_bmpTabSelMask;
	CBitmap m_bmpTabBtnN;
	CBitmap m_bmpTabBtnH;
	CBitmap m_bmpTabBtnC;
	CBitmap m_bmpTabBtnI;

	PropTabStat m_PropStat;
	int m_iBaseX;
	int m_nGrabIndex;
	E_PROP_GRAB_TYPE m_eGType;

private:
	void GetBitmapList(BitmapLoadInfo** blist,int* num);
	void DeleteBitmaps();
	void DrawTab(CDrawer& drawer,int xpos,const CString& left,const CString& right,bool sel,E_PROP_BTN_STATE state);
	void DrawTab(CDrawer& drawer,PropTabData* tab,int xpos);

	int DetectGrabState(LPPOINT pt,bool mousedown,E_PROP_GRAB_TYPE& type);

private:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnFileNewTab();
};
#endif