#include "pch.h"
#include "DrawObject.h"
#include "resource.h"
#include <assert.h>
#define clear_mem(m) memset(&m,0,sizeof(m))
#define DEFAULT_FONT _T("Times New Roman")
class StaticDrawResource
{
private:
	CBitmap m_bmpDotH;
	CBitmap m_bmpDotV;
	int m_HX;
	int m_VY;
	bool m_bInited;
public:
	StaticDrawResource():m_HX(-1),m_VY(-1),m_bInited(false)
	{
	}
	~StaticDrawResource()
	{
	}
	void Load()
	{
		if(m_bInited)
			return;
		m_bmpDotH.LoadBitmap(IDB_BMP_DOT_H);
		m_bmpDotV.LoadBitmap(IDB_BMP_DOT_V);
		BITMAP bm;
		m_bmpDotH.GetBitmap(&bm);
		m_HX=bm.bmWidth;
		m_bmpDotV.GetBitmap(&bm);
		m_VY=bm.bmHeight;
		m_bInited=true;
	}
	void Unload()
	{
		if(!m_bInited)
			return;
		m_bmpDotH.DeleteObject();
		m_bmpDotV.DeleteObject();
		m_HX=-1;
		m_VY=-1;
		m_bInited=false;
	}
	void DrawHDotLine(CDrawer* pDraw,POINT* pt,int length)
	{
		if((!m_bInited)||length<0)
			return;
		CPoint coord(*pt);
		int end=coord.x+length;
		for(int pos=coord.x;pos<end;)
		{
			int drawend=pos+m_HX;
			if(drawend>end)
				drawend=end;
			pDraw->DrawBitmap(&m_bmpDotH,&CPoint(pos,coord.y),SRCAND,&CRect(0,0,drawend-pos,1));
			pos=drawend;
		}
	}
	void DrawVDotLine(CDrawer* pDraw,POINT* pt,int length)
	{
		if((!m_bInited)||length<0)
			return;
		CPoint coord(*pt);
		int end=coord.y+length;
		for(int pos=coord.y;pos<end;)
		{
			int drawend=pos+m_VY;
			if(drawend>end)
				drawend=end;
			pDraw->DrawBitmap(&m_bmpDotV,&CPoint(coord.x,pos),SRCAND,&CRect(0,0,1,drawend-pos));
			pos=drawend;
		}
	}
};
static StaticDrawResource s_DRes;
void DrawObjectStartup()
{
	s_DRes.Load();
}
void DrawObjectShutdown()
{
	s_DRes.Unload();
}
enum E_DRAWTYPE
{
	eDrawDirect=1,
	eDrawBuffered,
	eDrawBufferedNoCreateDC,
};
class DrawObject
{
public:
	DrawObject(CDC* pDC):m_pDC(pDC),m_pDrawObj(NULL),m_pOldObj(NULL)
	{
	}
	void Init()
	{
		m_pOldObj=m_pDC->SelectObject(m_pDrawObj);
	}
	virtual CGdiObject* GetDrawObject()=0;
	virtual ~DrawObject()
	{
		if(m_pDC!=NULL&&m_pOldObj!=NULL)
		{
			m_pDC->SelectObject(m_pOldObj);
		}
		if(m_pDrawObj!=NULL)
		{
			m_pDrawObj->DeleteObject();
			delete m_pDrawObj;
		}
	}
protected:
	CGdiObject* m_pDrawObj;
	CGdiObject* m_pOldObj;
	CDC* m_pDC;
};
class DrawPen:public DrawObject
{
public:
	DrawPen(CDC* pDC,COLORREF clr,int width,int style):DrawObject(pDC)
	{
		m_pDrawObj=new CPen;
		((CPen*)m_pDrawObj)->CreatePen(style,width,clr);
		Init();
	}
	virtual ~DrawPen()
	{
	}
	virtual CGdiObject* GetDrawObject()
	{
		return m_pDrawObj;
	}
private:
};
class DrawBrush:public DrawObject
{
public:
	DrawBrush(CDC* pDC,COLORREF clr):DrawObject(pDC)
	{
		m_pDrawObj=new CBrush;
		((CBrush*)m_pDrawObj)->CreateSolidBrush(clr);
		Init();
	}
	virtual ~DrawBrush()
	{
	}
	virtual CGdiObject* GetDrawObject()
	{
		return m_pDrawObj;
	}
private:
};
class DrawFont:public DrawObject
{
public:
	DrawFont(CDC* pDC,int height,LPCTSTR ftname=NULL):DrawObject(pDC)
	{
		LOGFONT logfont;
		clear_mem(logfont);
		logfont.lfHeight=height;
		logfont.lfCharSet=ANSI_CHARSET;
		_tcscpy_s(logfont.lfFaceName,ftname==NULL?DEFAULT_FONT:ftname);
		m_pDrawObj=new CFont;
		((CFont*)m_pDrawObj)->CreateFontIndirect(&logfont);
		Init();
	}
	virtual ~DrawFont()
	{
	}
	virtual CGdiObject* GetDrawObject()
	{
		return m_pDrawObj;
	}
private:
};
CDCDraw::CDCDraw(CWnd* pWnd,CDC* pClientDC,bool buffered):
	m_pMemDC(NULL),m_pClientDC(NULL),
	m_pBmpMem(NULL),m_pOldBmp(NULL),
	m_nType(0)
{
	ASSERT(pWnd!=NULL);
	CPoint ptOrg(0,0);
	pWnd->GetClientRect(&m_rcClient);
	if(pClientDC==NULL)
		m_nType=eDrawBuffered;
	else
	{
		m_pClientDC=pClientDC;
		ptOrg=m_pClientDC->GetViewportOrg();
		m_rcClient.MoveToXY(-ptOrg);
		if(buffered)
			m_nType=eDrawBufferedNoCreateDC;
		else
		{
			m_nType=eDrawDirect;
			return;
		}
	}
	if(m_pClientDC==NULL)
		m_pClientDC=new CClientDC(pWnd);
	m_pMemDC=new CDC;
	m_pMemDC->CreateCompatibleDC(m_pClientDC);
	m_pBmpMem=new CBitmap;
	m_pBmpMem->CreateCompatibleBitmap(m_pClientDC,m_rcClient.Width(),m_rcClient.Height());
	m_pOldBmp=m_pMemDC->SelectObject(m_pBmpMem);
	m_pMemDC->SetViewportOrg(ptOrg);
	m_pMemDC->FillSolidRect(&m_rcClient,RGB(255,255,255));
}
CDCDraw::~CDCDraw()
{
	switch(m_nType)
	{
	case eDrawDirect:
		break;
	case eDrawBuffered:
	case eDrawBufferedNoCreateDC:
		m_pClientDC->BitBlt(m_rcClient.left,m_rcClient.top,m_rcClient.Width(),m_rcClient.Height(),
			m_pMemDC,m_rcClient.left,m_rcClient.top,SRCCOPY);
		m_pMemDC->SelectObject(m_pOldBmp);
		m_pMemDC->DeleteDC();
		m_pBmpMem->DeleteObject();
		delete m_pMemDC;
		delete m_pBmpMem;
		if(m_nType!=eDrawBufferedNoCreateDC)
			delete (CClientDC*)m_pClientDC;
		break;
	default:
		ASSERT(FALSE);
	}
}
CDrawer::CDrawer(CDCDraw* canvas):m_pCanvas(canvas)
{
	switch(m_pCanvas->m_nType)
	{
	case eDrawDirect:
		m_pDCDraw=m_pCanvas->m_pClientDC;
		break;
	case eDrawBuffered:
	case eDrawBufferedNoCreateDC:
		m_pDCDraw=m_pCanvas->m_pMemDC;
		break;
	default:
		ASSERT(FALSE);
		m_pDCDraw=NULL;
	}
}
CDrawer::~CDrawer()
{
	RestoreClipRect();
}
void CDrawer::RestoreClipRect()
{
	VERIFY(SelectDC()->SelectClipRgn(NULL)!=ERROR);
}
void CDrawer::SetClipRect(LPCRECT rcclip)
{
	RestoreClipRect();
	if(rcclip==NULL)
		return;
	CPoint ptOrg=SelectDC()->GetViewportOrg();
	CRect clip=rcclip;
	clip.OffsetRect(ptOrg);
	CRgn rgn;
	rgn.CreateRectRgn(clip.left,clip.top,
		clip.right,clip.bottom);
	VERIFY(SelectDC()->SelectClipRgn(&rgn)!=ERROR);
}
inline CDC* CDrawer::SelectDC()
{
	return m_pDCDraw;
}
void CDrawer::DrawLine(POINT* start,POINT* end,COLORREF clr,int width,int style)
{
	if(width==1&&style==PS_DOT)
	{
		CPoint ptStart(*start),ptEnd(*end);
		if(ptStart.y==ptEnd.y) //Draw horizontal dot line
		{
			if(ptStart.x>ptEnd.x)
				swap(ptStart,ptEnd);
			s_DRes.DrawHDotLine(this,&ptStart,ptEnd.x-ptStart.x);
			return;
		}
		else if (ptStart.x == ptEnd.x) //Draw vertical dot line
		{
			if(ptStart.y>ptEnd.y)
				swap(ptStart,ptEnd);
			s_DRes.DrawVDotLine(this,&ptStart,ptEnd.y-ptStart.y);
			return;
		}
	}
	DrawPen pen(SelectDC(),clr,width,style);
	SelectDC()->MoveTo(*start);
	SelectDC()->LineTo(*end);
}
void CDrawer::DrawEllipse(RECT* rc,COLORREF clr,int linew)
{
	DrawPen pen(SelectDC(),clr,linew,PS_SOLID);
	SelectDC()->SelectStockObject(NULL_BRUSH);
	SelectDC()->Ellipse(rc);
}
void CDrawer::FillEllipse(RECT* rc,COLORREF clr)
{
	DrawBrush brush(SelectDC(),clr);
	SelectDC()->SelectStockObject(NULL_PEN);
	SelectDC()->Ellipse(rc);
}
void DrawDotRect(CDrawer* pDraw,RECT* rc)
{
	CRect rect=*(CRect*)rc;
	if(rect.left>rect.right)
		swap(rect.left,rect.right);
	if(rect.top>rect.bottom)
		swap(rect.top,rect.bottom);
	s_DRes.DrawHDotLine(pDraw,&rect.TopLeft(),rect.Width());
	s_DRes.DrawVDotLine(pDraw,&rect.TopLeft(),rect.Height());
	s_DRes.DrawHDotLine(pDraw,&CPoint(rect.left,rect.bottom),rect.Width());
	s_DRes.DrawVDotLine(pDraw,&CPoint(rect.right,rect.top),rect.Height());
}
void CDrawer::DrawRect(RECT* rc,COLORREF clr,int linew,int style)
{
	if(linew==1&&style==PS_DOT)
	{
		DrawDotRect(this,rc);
	}
	else
	{
		DrawPen pen(SelectDC(),clr,linew,style);
		SelectDC()->SelectStockObject(NULL_BRUSH);
		SelectDC()->Rectangle(rc);
	}
}
void CDrawer::FillRect(RECT* rc,COLORREF clr)
{
	DrawBrush brush(SelectDC(),clr);
	SelectDC()->SelectStockObject(NULL_PEN);
	SelectDC()->Rectangle(rc);
}
void CDrawer::DrawBitmap(CBitmap* pBmp,POINT* pt,DWORD dwOps,RECT* srcrc)
{
	BITMAP bm;
	pBmp->GetBitmap(&bm);
	CRect srcrect(0,0,bm.bmWidth,bm.bmHeight);
	if(srcrc!=NULL)
	{
		srcrect=*srcrc;
		if(srcrect.right>bm.bmWidth)
			srcrect.right=bm.bmWidth;
		if(srcrect.bottom>bm.bmHeight)
			srcrect.bottom=bm.bmHeight;
	}
	CDC dcBmp;
	dcBmp.CreateCompatibleDC(m_pCanvas->m_pClientDC);
	CBitmap* oldbmp=dcBmp.SelectObject(pBmp);
	SelectDC()->BitBlt(pt->x,pt->y,srcrect.Width(),srcrect.Height(),
		&dcBmp,srcrect.left,srcrect.top,dwOps);
	dcBmp.SelectObject(oldbmp);
	dcBmp.DeleteDC();
}
void CDrawer::DrawText(POINT* pos,LPCTSTR text,int height,COLORREF clr,UINT backmode,LPCTSTR ftname)
{
	DrawFont font(SelectDC(),height,ftname);
	SelectDC()->SetTextColor(clr);
	SelectDC()->SetBkMode(backmode);
	SelectDC()->TextOut(pos->x,pos->y,text);
}
void CDrawer::DrawText(LPCRECT rect,UINT align,LPCTSTR text,int height,COLORREF clr,UINT backmode,LPCTSTR ftname)
{
	CString txt(text);
	CSize txtsize=GetTextExtent(txt,height,ftname);
	if(txtsize.cx>((CRect*)rect)->Width())
	{
		CString tmptxt=txt;
		while((!tmptxt.IsEmpty())&&txtsize.cx>((CRect*)rect)->Width())
		{
			tmptxt=tmptxt.Left(tmptxt.GetLength()-1);
			txt=tmptxt+_T("...");
			txtsize=GetTextExtent(txt,height,ftname);
		}
		if(tmptxt.IsEmpty())
			txt.Empty();
	}
	CPoint pt;
	if(align&DT_ALIGN_LEFT)
		pt=CPoint(rect->left,rect->top+(((CRect*)rect)->Height()-height)/2);
	else if(align&DT_ALIGN_RIGHT)
		pt=CPoint(rect->right-txtsize.cx,rect->top+(((CRect*)rect)->Height()-height)/2);
	DrawText(&pt,txt,height,clr,backmode,ftname);
}
CSize CDrawer::GetTextExtent(LPCTSTR text,int height,LPCTSTR ftname)
{
	DrawFont font(SelectDC(),height,ftname);
	return SelectDC()->GetTextExtent(text,(int)_tcslen(text));
}