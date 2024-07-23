#include "pch.h"
#include "MyToolBar.h"
#include "DrawObject.h"
struct CToolBarData
{
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	//WORD aItems[wItemCount]

	WORD* items()
		{ return (WORD*)(this+1); }
};
CMyToolBar::CMyToolBar():CToolBar()
{
	m_pData=NULL;
}
CMyToolBar::~CMyToolBar()
{
	if (m_pData!=NULL)
		delete[] m_pData;
}
BOOL CMyToolBar::LoadToolBar(LPCTSTR lpszResourceName,const CString& strInfo)
{
	ASSERT_VALID(this);
	ASSERT(lpszResourceName != NULL);

	// determine location of the bitmap in resource fork
	HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_TOOLBAR);
	HRSRC hRsrc = ::FindResource(hInst, lpszResourceName, RT_TOOLBAR);
	if (hRsrc == NULL)
		return FALSE;

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)
		return FALSE;

	CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
	if (pData == NULL)
		return FALSE;
	ASSERT(pData->wVersion == 1);

	UINT* pItems = new UINT[pData->wItemCount];
	for (int i = 0; i < pData->wItemCount; i++)
		pItems[i] = pData->items()[i];
	BOOL bResult = SetButtons(pItems, pData->wItemCount);
	if(!bResult)
	{
		delete[] pItems;
		goto end;
	}
	m_pData=new MyToolBarData[m_nCount];
	memset(m_pData,0,m_nCount*sizeof(MyToolBarData));
	for(int i=0;i<m_nCount;i++)
	{
		m_pData[i].nID=pItems[i];
	}
	delete[] pItems;

	// set new sizes of the buttons
	{
		CSize sizeImage(pData->wWidth, pData->wHeight);
		CSize sizeButton(pData->wWidth + 7, pData->wHeight + 7);
		SetSizes(CProgramData::GetRealSize(sizeButton), CProgramData::GetRealSize(sizeImage));
	}

	// load bitmap now that sizes are known by the toolbar control
	bResult=m_bmpButton.LoadBitmap(lpszResourceName);

end:
	UnlockResource(hGlobal);
	FreeResource(hGlobal);

	return bResult;
}


BEGIN_MESSAGE_MAP(CMyToolBar, CToolBar)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


void CMyToolBar::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CToolBar::OnPaint() for painting messages
	CDCDraw canvas(this,&dc,true);
	CDrawer drawer(&canvas);
	BITMAP bm;
	m_bmpButton.GetBitmap(&bm);
	CRect rc(0,0,bm.bmWidth,bm.bmHeight),rcWnd;
	rc=CProgramData::GetRealRect(rc);
	GetWindowRect(&rcWnd);
	int offset=(rcWnd.Height()-rc.Height())/2;
	rc.OffsetRect(offset,offset);
	drawer.DrawBitmapScaled(&m_bmpButton,&rc);
}


BOOL CMyToolBar::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return CToolBar::OnEraseBkgnd(pDC);
}


void CMyToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CToolBar::OnLButtonDown(nFlags, point);
}


void CMyToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CToolBar::OnLButtonUp(nFlags, point);
}


void CMyToolBar::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CToolBar::OnMouseMove(nFlags, point);
}


void CMyToolBar::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default

	CToolBar::OnMouseLeave();
}


void CMyToolBar::OnDestroy()
{
	CToolBar::OnDestroy();

	// TODO: Add your message handler code here
	m_bmpButton.DeleteObject();
}
