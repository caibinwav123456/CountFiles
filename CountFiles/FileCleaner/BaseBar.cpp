// BaseBar.cpp : implementation file
//

#include "pch.h"
#include "resource.h"
#include "BaseBar.h"


// CBaseBar dialog

IMPLEMENT_DYNAMIC(CBaseBar, CDialog)

CBaseBar::CBaseBar(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOGBAR, pParent),m_comboBasePath(eBComboMain), m_comboBasePath2(eBComboRef)
{

}

CBaseBar::~CBaseBar()
{
}

BOOL CBaseBar::CreateBar(CWnd* pParentWnd)
{
	return Create(IDD_DIALOGBAR, pParentWnd);
}

void CBaseBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_BASE_PATH, m_comboBasePath);
	DDX_Control(pDX, IDC_BUTTON_GO, m_btnGo);
	DDX_Control(pDX, IDC_BUTTON_OPEN, m_btnOpen);
	DDX_Control(pDX, IDC_BUTTON_FOLD, m_btnFold);
	DDX_Control(pDX, IDC_COMBO_BASE_PATH2, m_comboBasePath2);
	DDX_Control(pDX, IDC_BUTTON_GO2, m_btnGo2);
	DDX_Control(pDX, IDC_BUTTON_OPEN2, m_btnOpen2);
	DDX_Control(pDX, IDC_BUTTON_FOLD2, m_btnFold2);
	DDX_Control(pDX, IDC_BUTTON_DFOLD, m_btnDFold);
}


BEGIN_MESSAGE_MAP(CBaseBar, CDialog)
	ON_MESSAGE(WM_ENABLE_BTN_GO, &CBaseBar::OnEnableBtnGo)
	ON_MESSAGE(WM_SIZEPARENT, &CBaseBar::OnSizeParent)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_GO, &CBaseBar::OnBnClickedButtonGo)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CBaseBar::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_FOLD, &CBaseBar::OnBnClickedButtonFold)
	ON_BN_CLICKED(IDC_BUTTON_GO2, &CBaseBar::OnBnClickedButtonGo2)
	ON_BN_CLICKED(IDC_BUTTON_OPEN2, &CBaseBar::OnBnClickedButtonOpen2)
	ON_BN_CLICKED(IDC_BUTTON_FOLD2, &CBaseBar::OnBnClickedButtonFold2)
	ON_BN_CLICKED(IDC_BUTTON_DFOLD, &CBaseBar::OnBnClickedButtonDfold)
END_MESSAGE_MAP()


// CBaseBar message handlers
LRESULT CBaseBar::OnSizeParent(WPARAM wParam, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;
	CRect rect,rcParent;
	GetWindowRect(&rect);
	rect.OffsetRect(-rect.TopLeft());
	rect.OffsetRect(lpLayout->rect.left,lpLayout->rect.top);
	CWnd* pParent=GetParent();
	pParent->GetClientRect(&rcParent);
	rect.right=rcParent.right;
	AfxRepositionWindow(lpLayout,m_hWnd,&rect);
	lpLayout->rect.top+=rect.Height();
	lpLayout->sizeTotal=CRect(lpLayout->rect).Size();
	return 0;
}

void CBaseBar::RelayoutBarCtrl(CRect* rc)
{

}

void CBaseBar::OnBnClickedButtonGo()
{
	// TODO: Add your control notification handler code here
	m_btnGo.EnableButton(FALSE);
}


void CBaseBar::OnBnClickedButtonOpen()
{
	// TODO: Add your control notification handler code here
}


void CBaseBar::OnBnClickedButtonFold()
{
	// TODO: Add your control notification handler code here
}


void CBaseBar::OnBnClickedButtonGo2()
{
	// TODO: Add your control notification handler code here
	m_btnGo2.EnableButton(FALSE);
}


void CBaseBar::OnBnClickedButtonOpen2()
{
	// TODO: Add your control notification handler code here
}


void CBaseBar::OnBnClickedButtonFold2()
{
	// TODO: Add your control notification handler code here
}


void CBaseBar::OnBnClickedButtonDfold()
{
	// TODO: Add your control notification handler code here
}


BOOL CBaseBar::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_btnGo.LoadBitmaps(IDB_BMP_GO_N, IDB_BMP_GO_C, IDB_BMP_GO_H, IDB_BMP_GO_D);
	m_btnGo.SizeToContent();
	m_btnOpen.LoadBitmaps(IDB_BMP_OPEN_N, IDB_BMP_OPEN_C, IDB_BMP_OPEN_H, IDB_BMP_OPEN_D);
	m_btnOpen.SizeToContent();
	m_btnFold.LoadBitmaps(IDB_BMP_FOLD_N, IDB_BMP_FOLD_C, IDB_BMP_FOLD_H, IDB_BMP_FOLD_D);
	m_btnFold.SizeToContent();

	m_btnGo.EnableButton(FALSE);

	m_btnGo2.LoadBitmaps(IDB_BMP_GO_N, IDB_BMP_GO_C, IDB_BMP_GO_H, IDB_BMP_GO_D);
	m_btnGo2.SizeToContent();
	m_btnOpen2.LoadBitmaps(IDB_BMP_OPEN_N, IDB_BMP_OPEN_C, IDB_BMP_OPEN_H, IDB_BMP_OPEN_D);
	m_btnOpen2.SizeToContent();
	m_btnFold2.LoadBitmaps(IDB_BMP_FOLD_N, IDB_BMP_FOLD_C, IDB_BMP_FOLD_H, IDB_BMP_FOLD_D);
	m_btnFold2.SizeToContent();

	m_btnGo2.EnableButton(FALSE);

	m_btnDFold.LoadBitmaps(IDB_BMP_DFOLD_N, IDB_BMP_DFOLD_C, IDB_BMP_DFOLD_H, IDB_BMP_DFOLD_D);
	m_btnDFold.SizeToContent();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CBaseBar::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialog::OnOK();
	m_btnGo.EnableButton(FALSE);
	m_btnGo2.EnableButton(FALSE);
}


void CBaseBar::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialog::OnCancel();
}


LRESULT CBaseBar::OnEnableBtnGo(WPARAM wParam, LPARAM lParam)
{
	switch(wParam)
	{
	case eBComboMain:
		m_btnGo.EnableButton(TRUE);
		break;
	case eBComboRef:
		m_btnGo2.EnableButton(TRUE);
		break;
	}
	return 0;
}


void CBaseBar::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	CRect rc(0,0,cx,cy);
	RelayoutBarCtrl(&rc);
}
