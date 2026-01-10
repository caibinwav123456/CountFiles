#pragma once
#define MTB_STYLE_PUSHBTN  0
#define MTB_STYLE_CHECK    1
#define MTB_STYLE_GROUPBTN 2
#define MTB_STYLE_DROPBTN  4
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
		ItemIterator(CMyToolBar* host,BOOL vert=FALSE,CRect* prcWnd=NULL);
		operator bool();
		void operator++(int);
		BOOL m_bVert;
		CSize m_szBtn;
		CSize m_szImg;
		CRect m_rcBtn;
		CRect m_rcDrop;
		CRect m_rcImg;
		CRect m_rcImgSrc;
		CRect m_rcWnd;
		INT m_nDropWidth;
		INT m_nExBtnWidthT;
		INT m_nBtnWidthT;
		INT m_nBtnHeightT;
		INT m_nExImgWidthT;
		INT m_nImgWidthT;
		INT m_nImgHeightT;
		CPoint m_ptBtnOffset;
		CPoint m_ptImgOffset;
		UINT m_nCnt;
		UINT m_idx;
		MyToolBarData* m_pData;
		bool IsDropDown(UINT idx)
		{
			return !!(m_pData[idx].style&MTB_STYLE_DROPBTN);
		}
	};
	CMyToolBar();
	~CMyToolBar();
	BOOL LoadToolBar(UINT nIDResource)
	{
		CString strInfo;
		strInfo.LoadString(nIDResource);
		return LoadToolBar(MAKEINTRESOURCE(nIDResource),strInfo);
	}
	BOOL LoadToolBar(LPCTSTR lpszResourceName,const CString& strInfo);
	void InitialDock(CFrameWnd* frame,BOOL bNewRow=FALSE);
	virtual CSize CalcDynamicLayout(int nLength, DWORD nMode);
protected:
	CSize GetButtonSize(){return m_sizeButton;}
	CSize GetImageSize(){return m_sizeImage;}
	UINT GetButtonCount(){return m_nCount;}
	BOOL IsVertical(){return !!(GetBarStyle()&(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT));}
private:
	CBitmap m_bmpButton;
	MyToolBarData* m_pData;
	INT m_nDropWidth;
	int m_nDropCnt;
	CSize m_szBtnOrg;
	CSize m_szImgOrg;

	BOOL ParseConfigString(LPCTSTR strInfo);
	void CalcSize(void* lpVoid);
	CSize GetBarSize();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();

	static CPoint s_ptBarTileOrg;
	static int s_nBarRowHeight;
};
