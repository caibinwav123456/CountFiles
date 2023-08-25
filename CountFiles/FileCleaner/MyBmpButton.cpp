#include "pch.h"
#include "MyBmpButton.h"

CMyBmpButton::CMyBmpButton()
{
	m_iBtnState=0;
}

BEGIN_MESSAGE_MAP(CMyBmpButton,CBitmapButton)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

void CMyBmpButton::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if(m_iBtnState==0)
	{
		m_iBtnState=ODS_FOCUS;
		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme,sizeof(tme));
		tme.cbSize=sizeof(TRACKMOUSEEVENT);
		tme.dwFlags=TME_LEAVE;
		tme.hwndTrack=m_hWnd;
		TrackMouseEvent(&tme);
		Invalidate(FALSE);
	}
	CBitmapButton::OnMouseMove(nFlags, point);
}


void CMyBmpButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO:  Add your code to draw the specified item
	lpDrawItemStruct->itemState = m_iBtnState;
	CBitmapButton::DrawItem(lpDrawItemStruct);
}


void CMyBmpButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_iBtnState=ODS_SELECTED;
	Invalidate(FALSE);
	CBitmapButton::OnLButtonDown(nFlags, point);
}


void CMyBmpButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_iBtnState=ODS_FOCUS;
	Invalidate(FALSE);
	CBitmapButton::OnLButtonUp(nFlags, point);
}


void CMyBmpButton::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default
	m_iBtnState=0;
	Invalidate(FALSE);
	CBitmapButton::OnMouseLeave();
}
