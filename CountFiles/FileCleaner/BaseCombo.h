#pragma once
#define WM_ENABLE_BTN_GO (WM_USER+2000)
class CBaseCombo :
	public CComboBox
{
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnEditchange();
	afx_msg void OnCbnEditupdate();
};