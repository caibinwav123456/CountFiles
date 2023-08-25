#pragma once
#include <afxext.h>
class CMyBmpButton :
	public CBitmapButton
{
public:
	CMyBmpButton();

	void EnableButton(BOOL bEnable);

private:
	UINT m_iBtnState;
	BOOL m_bDisabled;

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);

private:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
};