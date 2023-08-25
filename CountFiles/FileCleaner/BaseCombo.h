#pragma once
#define WM_ENABLE_BTN_GO (WM_USER+1000)
class CBaseCombo :
	public CComboBox
{
public:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnEditchange();
	afx_msg void OnCbnEditupdate();
};