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
}


BEGIN_MESSAGE_MAP(CBaseBar, CDialog)
	ON_MESSAGE(WM_SIZEPARENT, &CBaseBar::OnSizeParent)
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
	rect.right=rcParent.Width()-rect.left;
	AfxRepositionWindow(lpLayout,m_hWnd,&rect);
	lpLayout->rect.top+=rect.Height();
	lpLayout->sizeTotal=CRect(lpLayout->rect).Size();
	return 0;
}