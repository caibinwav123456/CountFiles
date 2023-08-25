// BaseBar.cpp : implementation file
//

#include "pch.h"
#include "resource.h"
#include "BaseBar.h"


// CBaseBar dialog

IMPLEMENT_DYNAMIC(CBaseBar, CDialog)

CBaseBar::CBaseBar(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOGBAR, pParent)
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
}


BEGIN_MESSAGE_MAP(CBaseBar, CDialog)
	ON_MESSAGE(WM_SIZEPARENT, &CBaseBar::OnSizeParent)
	ON_BN_CLICKED(IDC_BUTTON_GO, &CBaseBar::OnBnClickedButtonGo)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CBaseBar::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_FOLD, &CBaseBar::OnBnClickedButtonFold)
	ON_MESSAGE(WM_ENABLE_BTN_GO, &CBaseBar::OnEnableBtnGo)
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

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CBaseBar::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialog::OnOK();
	m_btnGo.EnableButton(FALSE);
}


void CBaseBar::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialog::OnCancel();
}


LRESULT CBaseBar::OnEnableBtnGo(WPARAM wParam, LPARAM lParam)
{
	m_btnGo.EnableButton(TRUE);
	return 0;
}
