#pragma once
#include <afxext.h>
enum E_MYBUTTON_STYLE
{
	eButtonNormal,
	eButtonDropMain,
	eButtonDropDown,
};
class CMyBmpButton :
	public CBitmapButton
{
	friend class DropDownButton;
public:
	CMyBmpButton();

	void EnableButton(BOOL bEnable);
	void RestoreButtonState();

private:
	UINT m_iBtnState;
	BOOL m_bDisabled;
	E_MYBUTTON_STYLE m_eStyle;
	DropDownButton* m_pBtnNotify;

	void SetStyle(E_MYBUTTON_STYLE style);
	void SetButtonNotify(DropDownButton* btn);

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);

public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
};

class DropDownButton
{
public:
	DropDownButton();

	CMyBmpButton m_btnMain;
	CMyBmpButton m_btnDrop;

public:
	void EnableButton(BOOL bEnable);
	void RestoreButtonState();

	void GetWindowRect(LPRECT pRect);
	void MoveWindow(LPRECT pRect);

private:
	UINT m_iCompState;
	BOOL m_bInWndRect;
	void UpdateButtonState();
	HWND get_btn_hwnd(E_MYBUTTON_STYLE style);

public:
	void OnMouseMove(CPoint point,E_MYBUTTON_STYLE style);
	void OnMouseLeave(E_MYBUTTON_STYLE style);
};

inline HWND DropDownButton::get_btn_hwnd(E_MYBUTTON_STYLE style)
{
	switch(style)
	{
	case eButtonDropMain:
		return m_btnMain.m_hWnd;
		break;
	case eButtonDropDown:
		return m_btnDrop.m_hWnd;
		break;
	default:
		ASSERT(FALSE);
		return 0;
	}
}
