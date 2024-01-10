#include "pch.h"
#include "PropWnd.h"
#include "defines.h"
#include "resource.h"
IMPLEMENT_DYNAMIC(CPropWnd,CWnd)
IMPLEMENT_ID2WND_MAP(CPropWnd,IDW_PROP_WND)
CPropWnd::CPropWnd()
{

}
CPropWnd::~CPropWnd()
{

}
BEGIN_MESSAGE_MAP(CPropWnd, CWnd)
	ON_MESSAGE(WM_SIZEPARENT, &CPropWnd::OnSizeParent)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


int CPropWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	if(!m_bmpBk.LoadBitmap(IDB_BMP_PROP_BACK))
		return -1;

	return 0;
}


BOOL CPropWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Add your specialized code here and/or call the base class
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS,
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}


void CPropWnd::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	delete this;
}

LRESULT CPropWnd::OnSizeParent(WPARAM wParam, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;
	CRect rect(0,0,0,PROP_WND_HEIGHT),rcParent;
	GetParent()->GetClientRect(&rcParent);
	rect.right=rcParent.right;
	AfxRepositionWindow(lpLayout,m_hWnd,&rect);
	lpLayout->rect.top+=rect.Height();
	lpLayout->sizeTotal=CRect(lpLayout->rect).Size();
	return 0;
}


void CPropWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CWnd::OnPaint() for painting messages
	CRect rcClient;
	GetClientRect(&rcClient);
	CDCDraw canvas(this,&dc,true);
	CDrawer drawer(&canvas);
	drawer.DrawBitmapScaled(&m_bmpBk,&rcClient,NULL,SRCCOPY);
}


void CPropWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	Invalidate();
}


BOOL CPropWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return TRUE;//CWnd::OnEraseBkgnd(pDC);
}


void CPropWnd::OnDestroy()
{
	CWnd::OnDestroy();

	// TODO: Add your message handler code here
	m_bmpBk.DeleteObject();
}
