#include "pch.h"
#include "MyToolBar.h"
#include "DrawObject.h"
#define SEPARATOR_COLOR     RGB(140,140,140)
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
#define for_each_item(iter) for(ItemIterator iter(this);iter;iter++)

CMyToolBar::CMyToolBar():CToolBar()
{
	m_pData=NULL;
	m_nDropWidth=0;
	m_bNMMsgHandle=FALSE;
}

CMyToolBar::~CMyToolBar()
{
	SAFE_DELETE_ARRAY(m_pData);
}

BOOL CMyToolBar::MyCreate(UINT nID,CFrameWnd* pParentFrm)
{
	if (!CreateEx(pParentFrm, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP |
		CBRS_BORDER_ANY | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
		CRect(0,0,0,0), nID))
		return FALSE;      // fail to create

	if(!LoadToolBar(nID))
		return FALSE;      // fail to create

	EnableDocking(CBRS_ALIGN_ANY);
	pParentFrm->EnableDocking(CBRS_ALIGN_ANY);
	InitialDock(pParentFrm);

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
}


void CMyToolBar::OnPaint()
{
	if(m_bDelayedButtonLayout)
		Layout();

	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CToolBar::OnPaint() for painting messages
	CDCDraw canvas(this,&dc,true);
	CDrawer drawer(&canvas);
	for_each_item(btn)
	{
		if(m_pData[btn.m_idx].nID!=0)
			drawer.DrawBitmapScaled(&m_bmpButton,&btn.m_rcImg,&btn.m_rcImgSrc);
		else
		{
			drawer.DrawRect(&btn.m_rcBtn,SEPARATOR_COLOR);
			drawer.FillRect(&btn.m_rcBtn,SEPARATOR_COLOR);
		}
	}
}


void CMyToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (LButtonDown(nFlags,point))
		return;
}


void CMyToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if(LButtonUp(nFlags,point))
		return;
}


void CMyToolBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if(MouseMove(nFlags,point))
		return;
}


void CMyToolBar::OnMouseLeave()
{
	if(MouseLeave())
		return;
}
