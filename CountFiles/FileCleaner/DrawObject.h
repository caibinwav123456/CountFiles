#ifndef _DRAW_OBJECT_H_
#define _DRAW_OBJECT_H_
void DrawObjectStartup();
void DrawObjectShutdown();
class CDCDraw
{
	friend class CDrawer;
public:
	CDCDraw(CWnd* pWnd,CDC* pClientDC=NULL,bool buffered=false);
	~CDCDraw();
private:
	CDC* m_pMemDC;
	CDC* m_pClientDC;
	CBitmap* m_pBmpMem;
	CBitmap* m_pOldBmp;
	CRect m_rcClient;
	int m_nType;
};
#define DT_ALIGN_LEFT  1
#define DT_ALIGN_RIGHT 2
class CDrawer
{
public:
	CDrawer(CDCDraw* canvas);
	~CDrawer();

	void SetClipRect(LPCRECT rcclip);

	void DrawLine(const POINT* start,const POINT* end,COLORREF clr,int width=1,int style=PS_SOLID);

	void DrawEllipse(LPCRECT rc,COLORREF clr=RGB(0,0,0),int linew=1);
	void FillEllipse(LPCRECT rc,COLORREF clr=RGB(0,0,0));

	void DrawRect(LPCRECT rc,COLORREF clr=RGB(0,0,0),int linew=1,int style=PS_SOLID);
	void FillRect(LPCRECT rc,COLORREF clr=RGB(0,0,0));

	void DrawRoundRect(LPCRECT rc,const POINT* r,COLORREF clr=RGB(0,0,0),int linew=1,int style=PS_SOLID);
	void FillRoundRect(LPCRECT rc,const POINT* r,COLORREF clr=RGB(0,0,0));

	void DrawBitmap(CBitmap* pBmp,const POINT* pt,DWORD dwOps=SRCCOPY,LPCRECT srcrc=NULL);
	void DrawBitmapScaled(CBitmap* pBmp,LPCRECT dstrc,LPCRECT srcrc=NULL,DWORD dwOps=SRCCOPY);

	void DrawText(const POINT* pos,LPCTSTR text,int height=24,COLORREF clr=RGB(0,0,0),UINT backmode=TRANSPARENT,LPCTSTR ftname=NULL);
	void DrawText(LPCRECT rect,UINT align,LPCTSTR text,int height=24,COLORREF clr=RGB(0,0,0),UINT backmode=TRANSPARENT,LPCTSTR ftname=NULL);
	CSize GetTextExtent(LPCTSTR text,int height=24,LPCTSTR ftname=NULL);
private:
	CDC* SelectDC();
	CDCDraw* m_pCanvas;
	CDC* m_pDCDraw;
	void RestoreClipRect();
};
#endif
