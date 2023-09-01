#pragma once
#include "BaseCombo.h"
#include "MyBmpButton.h"

struct BarRelayoutObject
{
	CRect rect;
	CWnd* wndCombo;
	CWnd* btnGo;
	DropDownButton* btnOpen;
	CWnd* btnFold;
	BarRelayoutObject(CWnd* combo,CWnd* go,DropDownButton* open,CWnd* fold)
	{
		wndCombo=combo,btnGo=go,btnOpen=open,btnFold=fold;
	}
};

// CBaseBar dialog

class CBaseBar : public CDialog
{
	DECLARE_DYNAMIC(CBaseBar)

public:
	CBaseBar(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CBaseBar();

public:
	BOOL CreateBar(CWnd* pParentWnd);
	void SetBackPathMaxCount(UINT nmax);

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOGBAR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//private functions
private:
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	void RelayoutBarCtrl(CRect* rc);
	void RelayoutCtrlGroup(BarRelayoutObject* layout);

	void UpdateBaseBackBuffer(LPCTSTR left,LPCTSTR right);

	//class data
private:
	CBaseCombo m_comboBasePath;
	CMyBmpButton m_btnGo;
	DropDownButton m_btnOpen;
	CMyBmpButton m_btnFold;
	CBaseCombo m_comboBasePath2;
	CMyBmpButton m_btnGo2;
	DropDownButton m_btnOpen2;
	CMyBmpButton m_btnFold2;
	CMyBmpButton m_btnDFold;
	CString m_strComboBasePath;
	CString m_strComboBasePathRef;

	BOOL m_bInited;
	UINT m_nBasePathBufLen;

	CString m_strBasePath;
	CString m_strBasePathRef;

	//message handlers
private:
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEnableBtnGo(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonGo();
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonDrop();
	afx_msg void OnBnClickedButtonFold();
	afx_msg void OnBnClickedButtonGo2();
	afx_msg void OnBnClickedButtonOpen2();
	afx_msg void OnBnClickedButtonDrop2();
	afx_msg void OnBnClickedButtonFold2();
	afx_msg void OnBnClickedButtonDfold();
	afx_msg void OnCbnSelchangeComboBasePath();
	afx_msg void OnCbnSelchangeComboBasePath2();

	DECLARE_MESSAGE_MAP()
};
