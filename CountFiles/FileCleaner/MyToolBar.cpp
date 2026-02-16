#include "pch.h"
#include "MyToolBar.h"
#include "DrawObject.h"
#define SAFE_DELETE(ptr) \
	if(ptr!=NULL) \
	{ \
		delete ptr; \
		ptr=NULL; \
	}
#define SAFE_DELETE_ARRAY(ptr) \
	if(ptr!=NULL) \
	{ \
		delete[] ptr; \
		ptr=NULL; \
	}

CMyToolBar::CMyToolBar():CToolBar()
{
	m_pData=NULL;
	m_nDropWidth=0;
}

CMyToolBar::~CMyToolBar()
{
	SAFE_DELETE_ARRAY(m_pData);
}

BOOL CMyToolBar::MyCreate(UINT nID,UINT nIDConf,UINT bmp,UINT bmpBack,CFrameWnd* pParentFrm,BOOL bNextRow)
{
	if (!CreateEx(pParentFrm, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP |
		CBRS_BORDER_ANY | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
		CRect(0,0,0,0), nID))
		return FALSE;

	if(!LoadToolBar(nID,nIDConf,bmp,bmpBack))
		return FALSE;

	EnableDocking(CBRS_ALIGN_ANY);
	pParentFrm->EnableDocking(CBRS_ALIGN_ANY);
	InitialDock(pParentFrm,bNextRow);

	return TRUE;
}

CSize CMyToolBar::CalcDynamicLayout(int nLength,DWORD dwMode)
{
	ConfigButtons(nLength,dwMode);
	return CToolBar::CalcDynamicLayout(nLength,dwMode);
}


BEGIN_MESSAGE_MAP(CMyToolBar, CToolBar)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()


void CMyToolBar::OnDestroy()
{
	CToolBar::OnDestroy();

	// TODO: Add your message handler code here
	m_bmpButton.DeleteObject();
	m_bmpButtonBack.DeleteObject();
}


void CMyToolBar::OnPaint()
{
	PrepareForDraw();

	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CToolBar::OnPaint() for painting messages
	CDCDraw canvas(this,&dc,true);
	CDrawer drawer(&canvas);
	for_each_item(btn)
	{
		if(m_pData[btn.m_idx].nID!=0)
		{
			CRect rcBtn=btn.m_rcBtn;
			if(m_pData[btn.m_idx].style&MTB_STYLE_DROPBTN)
			{
				CRect rcDrop=btn.m_rcDrop,rcCombine;
				rcDrop.DeflateRect(CRect(0,2,2,2));
				rcBtn.DeflateRect(CRect(2,2,0,2));
				rcCombine.UnionRect(rcDrop,rcBtn);
				drawer.FillRoundRect(&rcCombine,&CPoint(2,2),TOOLBAR_H_COLOR);
				drawer.DrawRoundRect(&rcCombine,&CPoint(2,2),TOOLBAR_H_E_COLOR);
				drawer.DrawLine(&rcDrop.TopLeft(),&rcBtn.BottomRight(),TOOLBAR_H_E_COLOR);
			}
			else
			{
				rcBtn.DeflateRect(CRect(2,2,2,2));
				drawer.FillRoundRect(&rcBtn,&CPoint(2,2),TOOLBAR_H_COLOR);
				drawer.DrawRoundRect(&rcBtn,&CPoint(2,2),TOOLBAR_H_E_COLOR);
			}
			drawer.DrawBitmapScaled(&m_bmpButtonBack,&btn.m_rcImg,&btn.m_rcImgSrc,SRCAND);
			drawer.DrawBitmapScaled(&m_bmpButton,&btn.m_rcImg,&btn.m_rcImgSrc,SRCPAINT);
		}
		else
		{
			drawer.DrawRect(&btn.m_rcBtn,SEPARATOR_COLOR);
			drawer.FillRect(&btn.m_rcBtn,SEPARATOR_COLOR);
		}
	}
}


void CMyToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(LButtonDown(nFlags,point))
		return;
	CWnd::OnLButtonDown(nFlags,point);
}


void CMyToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	CToolBar::OnLButtonUp(nFlags, point);
}


void CMyToolBar::OnMouseMove(UINT nFlags, CPoint point)
{
	CToolBar::OnMouseMove(nFlags, point);
}


void CMyToolBar::OnMouseLeave()
{
	CToolBar::OnMouseLeave();
}
