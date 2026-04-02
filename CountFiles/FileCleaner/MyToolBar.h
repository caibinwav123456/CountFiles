#pragma once
#include "DrawObject.h"
#define MTB_STYLE_PUSHBTN  0
#define MTB_STYLE_CHECK    1
#define MTB_STYLE_GROUPBTN 2
#define MTB_STYLE_DROPBTN  4
#define MTB_STYLE_PLACEHOLDER 0x10
#define MTB_STYLE_WRAP     0x20

#define MTB_STYLE_CUSTOM   MTB_STYLE_DROPBTN

#define for_each_item(iter) for(ItemIterator iter(this);iter;iter++)

enum E_MTB_STATE
{
	eMTBNormal,
	eMTBHighlight,
	eMTBClick,
	eMTBEXClick,
};
enum E_CHECK_OP
{
	eUncheck,
	eCheck,
	eInvChk,
};
struct AnimateSeg
{
	UINT m_nImgIndex;
	UINT m_nTime;
};
struct AnimateItem
{
	INT m_nSegCnt;
	AnimateSeg* m_pSegs;
};
struct AnimateData
{
	AnimateItem* m_pItems;
};
struct ImageIndex
{
	UINT m_idxNormal;
	UINT m_idxDisabled;
};
struct MyToolBarData
{
	UINT nID;
	INT menuidx;
	INT menucard;
	INT menucnt;
	UINT style;
	UINT state;
	BOOL checked;
	BOOL disabled;
	BOOL animating;
	MyToolBarData* pGrpNext;
	ImageIndex* m_pIcons;
	AnimateData* m_pAnims;
};
class CMyToolBar : public CToolBar
{
public:
	struct ItemIterator
	{
		ItemIterator(const CMyToolBar* host,CRect* prcWnd=NULL);
		operator bool();
		void operator++(int);
		BOOL m_bVert;
		CSize m_szBtn;
		CSize m_szImg;
		CRect m_rcBtn;
		CRect m_rcDrop;
		CRect m_rcImg;
		CRect m_rcImgSrc;
		CRect m_rcSep;
		CRect m_rcWrapSep;
		CRect m_rcWnd;
		CRect m_rcBtnOffset;
		CRect m_rcImgOffset;
		CPoint m_ptCur;
		INT m_nDropWidth;
		INT m_nExBtnWidthT;
		INT m_nBtnWidthT;
		INT m_nBtnHeightT;
		INT m_nExImgWidthT;
		INT m_nImgWidthT;
		INT m_nImgHeightT;
		INT m_nCnt;
		INT m_idx;
		INT m_iNext;
		MyToolBarData* m_pData;
		void CalcItemRect(BOOL bSep, BOOL bWrap, BOOL bDrop);
		void NextItem(BOOL bSep, BOOL bWrap, BOOL bDrop);
		void Iterate();
	};
	CMyToolBar();
	~CMyToolBar();
	BOOL MyCreate(UINT nID,UINT nIDConf,UINT bmp,UINT bmpBack,CFrameWnd* pParentFrm,BOOL bNextRow=FALSE);
	BOOL LoadToolBar(UINT nID,UINT nIDConf,UINT bmp,UINT bmpBack)
	{
		CString strInfo;
		strInfo.LoadString(nIDConf);
		return LoadToolBar(MAKEINTRESOURCE(nID),strInfo,MAKEINTRESOURCE(bmp),MAKEINTRESOURCE(bmpBack));
	}
	BOOL LoadToolBar(LPCTSTR lpszResourceName,LPCTSTR strInfo,LPCTSTR bmp,LPCTSTR bmpBack);
	void InitialDock(CFrameWnd* frame,BOOL bNewRow=FALSE);
	UINT GetButtonCount() const{return m_nBtnCnt;}
	BOOL CheckButton(int idx,int op);

	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode);
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

protected:
	CSize GetButtonSize() const{return m_sizeButton;}
	CSize GetImageSize() const{return m_sizeImage;}
	BOOL IsVertical() const{return !!((const_cast<CMyToolBar*>(this))->GetBarStyle()&(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT));}
	BOOL HitTest(CPoint pt);
	BOOL ItemFromPoint(const CPoint& pt,INT_PTR* nID,BOOL* bDrop,INT* idx=NULL,CRect* rect=NULL) const;
private:
	CBitmap m_bmpButton;
	CBitmap m_bmpButtonBack;
	MyToolBarData* m_pData;
	INT m_nBtnCnt;
	INT m_nBtnCntOrg;
	INT m_nDropWidth;
	INT m_iGrab;
	CSize m_szBtnOrg;
	CSize m_szImgOrg;
	BOOL ParseConfigString(LPCTSTR strInfo,WORD* pID,int cnt,int& outcnt);
	void ConfigButtons(int nLength, DWORD dwMode);
	void ConfigButton(int nLength,BOOL bVert=FALSE);
	int ArrangeButtons(void* lpVoid,int nCount,int len);
	void UpdateButtons();
	void CalcSize(void* lpVoid);
	CSize GetBarSize();
	void PrepareForDraw();
	void DrawItem(CDrawer& drawer,const ItemIterator& item);
	void DrawBtnBack(CDrawer& drawer,const ItemIterator& item);
	void DrawDropBtnBack(CDrawer& drawer,const ItemIterator& item);
	void DrawBtnImg(CDrawer& drawer,const ItemIterator& item);
	void RestoreBarState();

	BOOL LButtonDown(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();

	static CPoint s_ptBarTileOrg;
	static int s_nBarRowHeight;

protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};
