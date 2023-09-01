#include "pch.h"
#include "MyBmpButton.h"

CMyBmpButton::CMyBmpButton()
{
	m_iBtnState=0;
	m_bDisabled=FALSE;
	m_eStyle=eButtonNormal;
	m_pBtnNotify=NULL;
}

void CMyBmpButton::SetStyle(E_MYBUTTON_STYLE style)
{
	m_eStyle=style;
}

void CMyBmpButton::SetButtonNotify(DropDownButton* btn)
{
	m_pBtnNotify=btn;
}

DropDownButton::DropDownButton()
{
	m_iCompState=0;
	m_bInWndRect=FALSE;

	m_btnMain.SetStyle(eButtonDropMain);
	m_btnDrop.SetStyle(eButtonDropDown);

	m_btnMain.SetButtonNotify(this);
	m_btnDrop.SetButtonNotify(this);
}

BEGIN_MESSAGE_MAP(CMyBmpButton,CBitmapButton)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

void CMyBmpButton::EnableButton(BOOL bEnable)
{
	EnableWindow(bEnable);
	m_bDisabled=!bEnable;
	m_iBtnState=0;
	Invalidate(FALSE);
}

void DropDownButton::EnableButton(BOOL bEnable)
{
	m_btnMain.EnableButton(bEnable);
	m_btnDrop.EnableButton(bEnable);
}

void DropDownButton::GetWindowRect(LPRECT pRect)
{
	if(pRect==NULL)
		return;
	CRect rc1,rc2;
	m_btnMain.GetWindowRect(&rc1);
	m_btnDrop.GetWindowRect(&rc2);
	((CRect*)pRect)->UnionRect(&rc1,&rc2);
}
void DropDownButton::MoveWindow(LPRECT pRect)
{
	if(pRect==NULL)
		return;
	CRect rc;
	m_btnDrop.GetWindowRect(&rc);
	int width=rc.Width();
	CRect rcMain(pRect),rcDrop(pRect);
	rcMain.right-=width;
	rcDrop.left=rcMain.right;
	m_btnMain.MoveWindow(&rcMain);
	m_btnDrop.MoveWindow(&rcDrop);
}

void CMyBmpButton::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if(m_bDisabled)
		return;
	if(m_eStyle!=eButtonNormal)
		m_pBtnNotify->OnMouseMove(point,m_eStyle);
	else if(m_iBtnState==0)
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
	lpDrawItemStruct->itemState = (m_bDisabled?ODS_DISABLED:m_iBtnState);
	CBitmapButton::DrawItem(lpDrawItemStruct);
}


void CMyBmpButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if(m_bDisabled)
		return;
	m_iBtnState=ODS_SELECTED;
	Invalidate(FALSE);
	CBitmapButton::OnLButtonDown(nFlags, point);
}


void CMyBmpButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if(m_bDisabled)
		return;
	m_iBtnState=ODS_FOCUS;
	Invalidate(FALSE);
	CBitmapButton::OnLButtonUp(nFlags, point);
}


void CMyBmpButton::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default
	if(m_bDisabled)
		return;
	if(m_eStyle!=eButtonNormal)
		m_pBtnNotify->OnMouseLeave(m_eStyle);
	else
	{
		m_iBtnState=0;
		Invalidate(FALSE);
	}
	CBitmapButton::OnMouseLeave();
}

void DropDownButton::UpdateButtonState()
{
	m_btnMain.m_iBtnState=m_iCompState;
	m_btnDrop.m_iBtnState=m_iCompState;
	m_btnMain.Invalidate(FALSE);
	m_btnDrop.Invalidate(FALSE);
}

void DropDownButton::OnMouseMove(CPoint point,E_MYBUTTON_STYLE style)
{
	if(m_iCompState==0||m_bInWndRect)
	{
		m_iCompState=ODS_FOCUS;
		m_bInWndRect=FALSE;
		TRACKMOUSEEVENT tme;
		ZeroMemory(&tme,sizeof(tme));
		tme.cbSize=sizeof(TRACKMOUSEEVENT);
		tme.dwFlags=TME_LEAVE;
		tme.hwndTrack=get_btn_hwnd(style);
		TrackMouseEvent(&tme);
		UpdateButtonState();
	}
}

void DropDownButton::OnMouseLeave(E_MYBUTTON_STYLE style)
{
	CRect rc;
	GetWindowRect(&rc);
	CPoint ptCur;
	::GetCursorPos(&ptCur);
	if(!rc.PtInRect(ptCur))
		m_iCompState=0;
	else
	{
		m_iCompState=ODS_FOCUS;
		m_bInWndRect=TRUE;
	}
	UpdateButtonState();
}