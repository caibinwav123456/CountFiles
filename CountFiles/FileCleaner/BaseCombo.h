#pragma once
#define WM_ENABLE_BTN_GO (WM_USER+2000)
enum E_BASECOMBO_ID
{
	eBComboMain,
	eBComboRef,
};
class CBaseCombo :
	public CComboBox
{
public:
	CBaseCombo(E_BASECOMBO_ID id):m_eComboID(id){}
private:
	E_BASECOMBO_ID m_eComboID;
private:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnEditchange();
	afx_msg void OnCbnEditupdate();
};