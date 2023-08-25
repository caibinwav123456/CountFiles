#pragma once
#include "MyBmpButton.h"

// CBaseBar dialog

class CBaseBar : public CDialog
{
	DECLARE_DYNAMIC(CBaseBar)

public:
	CBaseBar(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CBaseBar();

public:
	BOOL CreateBar(CWnd* pParentWnd);

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOGBAR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//message handlers
private:
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonGo();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonFold();
	CComboBox m_comboBasePath;
	CMyBmpButton m_btnGo;
	CMyBmpButton m_btnOpen;
	CMyBmpButton m_btnFold;
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
};
