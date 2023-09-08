#include "pch.h"
#include "BaseCombo.h"

BEGIN_MESSAGE_MAP(CBaseCombo, CComboBox)
	ON_CONTROL_REFLECT(CBN_EDITCHANGE, &CBaseCombo::OnCbnEditchange)
	ON_CONTROL_REFLECT(CBN_EDITUPDATE, &CBaseCombo::OnCbnEditupdate)
END_MESSAGE_MAP()


void CBaseCombo::OnCbnEditchange()
{
	// TODO: Add your control notification handler code here
	GetParent()->SendMessage(WM_ENABLE_BTN_GO,m_eComboID);
}


void CBaseCombo::OnCbnEditupdate()
{
	// TODO: Add your control notification handler code here
}
