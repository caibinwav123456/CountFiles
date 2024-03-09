#ifndef _PROP_WND_H_
#define _PROP_WND_H_
#include "DrawObject.h"
#include "CommonStruct.h"
#include <vector>
#include <set>
struct PropTabData
{
	int ctrl_idx;
	int tab_idx;
	bool issel;
	CString lTitle;
	CString rTitle;
	CString lPath;
	CString rPath;
	PropTabData* prev;
	PropTabData* next;
	PropTabData():ctrl_idx(-1),tab_idx(-1),issel(false),prev(this),next(this){}
	void LinkAfter(PropTabData* data);
	void Remove();
	void UpdateString(const string& left,const string& right);
};
struct PropTabStat
{
	vector<PropTabData*> vecData;
	PropTabData first;
	PropTabData last;
	set<int> indices;
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
	void NewTab(const string& left="",const string& right="");
	int DeleteTab(int idx,int& next);
	bool ReorderTab(int origin, int insert_before);
	int SelectTab(int idx,bool& unchanged);
	void SetCurTabString(const string& left,const string& right="");
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
	eGrabMove,
	eGrabHoverMove,
};
class CPropWnd:public CWnd
{
	DECLARE_DYNAMIC(CPropWnd)
	DECLARE_ID2WND_MAP(CPropWnd)
public:
	CPropWnd();
	~CPropWnd();

	int GetPropCount();

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

	CBitmap m_bmpTabLeftN;
	CBitmap m_bmpTabLeftH;
	CBitmap m_bmpTabLeftC;
	CBitmap m_bmpTabLeftD;
	CBitmap m_bmpTabRightN;
	CBitmap m_bmpTabRightH;
	CBitmap m_bmpTabRightC;
	CBitmap m_bmpTabRightD;

	PropTabStat m_PropStat;
	int m_iBaseX;
	int m_iShiftTab;
	int m_nGrabIndex;
	BOOL m_bShowMove;
	E_PROP_GRAB_TYPE m_eGType;

	int m_nCancelMMove;

private:
	void GetBitmapList(BitmapLoadInfo** blist,int* num);
	void DeleteBitmaps();
	void DrawTab(CDrawer& drawer,int xpos,int tabidx,const CString& left,const CString& right,bool sel,E_PROP_BTN_STATE state);
	void DrawTab(CDrawer& drawer,PropTabData* tab,int xpos);
	void DrawMoveBtn(CDrawer& drawer);

	int DetectGrabState(LPPOINT pt,bool mousedown,E_PROP_GRAB_TYPE& type);
	int DetectMoveBtnState(LPPOINT pt,bool mousedown,E_PROP_GRAB_TYPE& type);
	void AdjustMoveBtn(int width=0);
	void AlignNewTab();
	void GetMoveBtnRect(RECT* rcl,RECT* rcr);
	void GetMoveBtnEnableState(bool& ldisable,bool& rdisable);

private:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PostNcDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetCurTitle(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseCurrentSession(WPARAM wParam, LPARAM lParam);
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