#pragma once
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
	eMTBDisable,
};
struct MyToolBarData
{
	UINT nID;
	UINT style;
	UINT state;
	BOOL checked;
	MyToolBarData* pGrpNext;
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
		UINT m_nCnt;
		UINT m_idx;
		UINT m_iNext;
		MyToolBarData* m_pData;
		void CalcItemRect(BOOL bSep, BOOL bWrap, BOOL bDrop);
		void NextItem(BOOL bSep, BOOL bWrap, BOOL bDrop);
		void Iterate();
	};
	CMyToolBar();
	~CMyToolBar();
	BOOL MyCreate(UINT nID,CFrameWnd* pParentFrm,BOOL bNextRow=FALSE);
	BOOL LoadToolBar(UINT nIDResource)
	{
		CString strInfo;
		strInfo.LoadString(nIDResource);
		return LoadToolBar(MAKEINTRESOURCE(nIDResource),strInfo);
	}
	BOOL LoadToolBar(LPCTSTR lpszResourceName,const CString& strInfo);
	void InitialDock(CFrameWnd* frame,BOOL bNewRow=FALSE);
	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode);
	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
protected:
	CSize GetButtonSize() const{return m_sizeButton;}
	CSize GetImageSize() const{return m_sizeImage;}
	UINT GetButtonCount() const{return m_nCount;}
	BOOL IsVertical() const{return !!((const_cast<CMyToolBar*>(this))->GetBarStyle()&(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT));}
	BOOL HitTest(CPoint pt);
	BOOL ItemFromPoint(const CPoint& pt,INT_PTR* nID,BOOL* bDrop,CRect* rect=NULL) const;
private:
	CBitmap m_bmpButton;
	MyToolBarData* m_pData;
	INT m_nDropWidth;
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
};
