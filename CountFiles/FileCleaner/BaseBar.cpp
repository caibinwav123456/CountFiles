// BaseBar.cpp : implementation file
//

#include "pch.h"
#include "resource.h"
#include "BaseBar.h"
#include "common.h"

#define ACC_PATH_TYPE_FILE 1
#define ACC_PATH_TYPE_DIR  2

// CBaseBar dialog

IMPLEMENT_DYNAMIC(CBaseBar, CDialog)
IMPLEMENT_ID2WND_MAP(CBaseBar,IDW_BASE_BAR)

CBaseBar::CBaseBar(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOGBAR, pParent)
	, m_comboBasePath(eBComboMain)
	, m_comboBasePath2(eBComboRef)
	, m_strComboBasePath(_T(""))
	, m_strComboBasePathRef(_T(""))
{
	m_bInited=FALSE;
	m_nBasePathBufLen=100;
}

CBaseBar::~CBaseBar()
{
}

BOOL CBaseBar::CreateBar(CWnd* pParentWnd)
{
	return Create(IDD_DIALOGBAR, pParentWnd);
}

void CBaseBar::SetBackPathMaxCount(UINT nmax)
{
	if(nmax>1000)
		nmax=1000;
	m_nBasePathBufLen=nmax;
}

void CBaseBar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_BASE_PATH, m_comboBasePath);
	DDX_Control(pDX, IDC_BUTTON_GO, m_btnGo);
	DDX_Control(pDX, IDC_BUTTON_OPEN, m_btnOpen.m_btnMain);
	DDX_Control(pDX, IDC_BUTTON_DROP, m_btnOpen.m_btnDrop);
	DDX_Control(pDX, IDC_BUTTON_FOLD, m_btnFold);
	DDX_Control(pDX, IDC_COMBO_BASE_PATH2, m_comboBasePath2);
	DDX_Control(pDX, IDC_BUTTON_GO2, m_btnGo2);
	DDX_Control(pDX, IDC_BUTTON_OPEN2, m_btnOpen2.m_btnMain);
	DDX_Control(pDX, IDC_BUTTON_DROP2, m_btnOpen2.m_btnDrop);
	DDX_Control(pDX, IDC_BUTTON_FOLD2, m_btnFold2);
	DDX_Control(pDX, IDC_BUTTON_DFOLD, m_btnDFold);
	DDX_CBString(pDX, IDC_COMBO_BASE_PATH, m_strComboBasePath);
	DDX_CBString(pDX, IDC_COMBO_BASE_PATH2, m_strComboBasePathRef);
}


BEGIN_MESSAGE_MAP(CBaseBar, CDialog)
	ON_MESSAGE(WM_ENABLE_BTN_GO, &CBaseBar::OnEnableBtnGo)
	ON_MESSAGE(WM_SIZEPARENT, &CBaseBar::OnSizeParent)
	ON_MESSAGE(WM_SET_VIEW_SIZE, &CBaseBar::OnSizeView)
	ON_WM_EXITMENULOOP()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_GO, &CBaseBar::OnBnClickedButtonGo)
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CBaseBar::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_DROP, &CBaseBar::OnBnClickedButtonDrop)
	ON_BN_CLICKED(IDC_BUTTON_FOLD, &CBaseBar::OnBnClickedButtonFold)
	ON_BN_CLICKED(IDC_BUTTON_GO2, &CBaseBar::OnBnClickedButtonGo2)
	ON_BN_CLICKED(IDC_BUTTON_OPEN2, &CBaseBar::OnBnClickedButtonOpen2)
	ON_BN_CLICKED(IDC_BUTTON_DROP2, &CBaseBar::OnBnClickedButtonDrop2)
	ON_BN_CLICKED(IDC_BUTTON_FOLD2, &CBaseBar::OnBnClickedButtonFold2)
	ON_BN_CLICKED(IDC_BUTTON_DFOLD, &CBaseBar::OnBnClickedButtonDfold)
	ON_CBN_SELCHANGE(IDC_COMBO_BASE_PATH, &CBaseBar::OnCbnSelchangeComboBasePath)
	ON_CBN_SELCHANGE(IDC_COMBO_BASE_PATH2, &CBaseBar::OnCbnSelchangeComboBasePath2)
	ON_COMMAND(ID_CMD_MENU_OPEN, &CBaseBar::OnCmdMenuOpenDir)
	ON_COMMAND(ID_CMD_MENU_IMP_FILE, &CBaseBar::OnCmdMenuImpFile)
	ON_COMMAND(ID_CMD_MENU_EXPRECFILE, &CBaseBar::OnCmdMenuExpRecFile)
	ON_COMMAND(ID_CMD_MENU_SELECT_REC, &CBaseBar::OnCmdMenuSelectRec)
	ON_COMMAND(ID_CMD_MENU_IMP_FILE_REF, &CBaseBar::OnCmdMenuImpFileRef)
	ON_COMMAND(ID_CMD_MENU_EXPRECFILE_REF, &CBaseBar::OnCmdMenuExpRecFileRef)
END_MESSAGE_MAP()


// CBaseBar message handlers
LRESULT CBaseBar::OnSizeParent(WPARAM wParam, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* lpLayout = (AFX_SIZEPARENTPARAMS*)lParam;
	CRect rect,rcParent;
	GetWindowRect(&rect);
	rect.OffsetRect(-rect.TopLeft());
	rect.OffsetRect(lpLayout->rect.left,lpLayout->rect.top);
	GetParent()->GetClientRect(&rcParent);
	rect.right=rcParent.right;
	AfxRepositionWindow(lpLayout,m_hWnd,&rect);
	lpLayout->rect.top+=rect.Height();
	lpLayout->sizeTotal=CRect(lpLayout->rect).Size();
	return 0;
}

LRESULT CBaseBar::OnSizeView(WPARAM wParam, LPARAM lParam)
{
	CRect rc=*(CRect*)wParam;
	if(m_bInited)
		RelayoutBarCtrl(&rc);
	return 0;
}

void CBaseBar::RelayoutBarCtrl(CRect* rc)
{
	CRect rcDFold,rcClient;
	BarRelayoutObject layout1(&m_comboBasePath,&m_btnGo,&m_btnOpen,&m_btnFold),
		layout2(&m_comboBasePath2,&m_btnGo2,&m_btnOpen2,&m_btnFold2);
	GetClientRect(&rcClient);
	ASSERT(rcClient.left==0&&rcClient.top==0);
	m_btnDFold.GetWindowRect(&rcDFold);
	rcDFold.OffsetRect(-rcDFold.TopLeft());
	int grpwidth=(max(MIN_SCROLL_WIDTH,rc->Width())-BAR_CENTER_SPACE)/2;
	rcDFold.OffsetRect(grpwidth+(BAR_CENTER_SPACE-rcDFold.Width())/2,(rcClient.Height()-rcDFold.Height())/2);
	m_btnDFold.MoveWindow(&rcDFold);
	layout1.rect=CRect(0,0,grpwidth,rcClient.bottom);
	layout2.rect=CRect(grpwidth+BAR_CENTER_SPACE,0,grpwidth*2+BAR_CENTER_SPACE,rcClient.bottom);
	RelayoutCtrlGroup(&layout1);
	RelayoutCtrlGroup(&layout2);
	Invalidate(FALSE);
}

void CBaseBar::RelayoutCtrlGroup(BarRelayoutObject* layout)
{
	CRect rc1,rc2,rc3,rcCB;
	layout->btnGo->GetWindowRect(&rc1);
	ScreenToClient(&rc1);
	layout->btnOpen->GetWindowRect(&rc2);
	ScreenToClient(&rc2);
	layout->btnFold->GetWindowRect(&rc3);
	ScreenToClient(&rc3);
	layout->wndCombo->GetWindowRect(&rcCB);
	ScreenToClient(&rcCB);
	CPoint offset(layout->rect.right-rc3.right-BAR_CENTER_MARGIN,0);
	rc1.OffsetRect(offset);
	rc2.OffsetRect(offset);
	rc3.OffsetRect(offset);
	rcCB.left=layout->rect.left+BAR_CENTER_MARGIN;
	rcCB.right=rc1.left-BAR_CENTER_MARGIN;
	rc1.OffsetRect(0,-rc1.top+(layout->rect.Height()-rc1.Height())/2);
	rc2.OffsetRect(0,-rc2.top+(layout->rect.Height()-rc2.Height())/2);
	rc3.OffsetRect(0,-rc3.top+(layout->rect.Height()-rc3.Height())/2);
	rcCB.OffsetRect(0,-rcCB.top+(layout->rect.Height()-rcCB.Height())/2);
	layout->wndCombo->MoveWindow(&rcCB);
	layout->btnGo->MoveWindow(rc1);
	layout->btnOpen->MoveWindow(rc2);
	layout->btnFold->MoveWindow(rc3);
}

void CBaseBar::RestoreCtrlState()
{
	m_btnOpen.RestoreButtonState();
	m_btnOpen2.RestoreButtonState();
}

CString CBaseBar::GetHandleFileName(const CString& path,BOOL bSave)
{
	TCHAR* strFileName = new TCHAR[65536];
	TCHAR* strFileTitle = new TCHAR[65536];
	dword type;
	if(bSave||path.IsEmpty()||sys_fstat((char*)t2a(path),&type)!=0||type!=FILE_TYPE_NORMAL)
	{
		_tcscpy_s(strFileName,65535,a2t(CProgramData::GetExportDirPath()));
		*strFileTitle=0;
	}
	else
	{
		_tcscpy_s(strFileName,65535,path);
		int pos=path.ReverseFind(_T('\\'));
		pos=(pos<0?0:pos+1);
		_tcscpy_s(strFileTitle,65535,((LPCTSTR)path)+pos);
	}
	CFileDialog dlg(!bSave, NULL, NULL, 0, _T("File List Files|*.fl||"), this);
	dlg.m_ofn.lpstrFile = strFileName;
	dlg.m_ofn.lpstrFileTitle = strFileTitle;
	dlg.m_ofn.nMaxFile = 65536;
	dlg.m_ofn.nMaxFileTitle = 65536;
	CString strImpFile;
	if(dlg.DoModal()==IDOK)
		strImpFile=dlg.GetPathName();
	delete[] strFileName;
	delete[] strFileTitle;
	return strImpFile;
}

inline int FindComboContent(CBaseCombo& combo,const CString& strItem)
{
	for(int i=0;i<combo.GetCount();i++)
	{
		CString str;
		combo.GetLBText(i,str);
		if(str==strItem)
			return i;
	}
	return -1;
}

inline void UpdateComboStrings(CBaseCombo& combo,const CString& str,UINT maxcnt)
{
	if(str.IsEmpty())
		return;
	int idx=FindComboContent(combo,str);
	if(idx>=0)
		combo.DeleteString(idx);
	combo.InsertString(0,str);
	while(combo.GetCount()>(int)maxcnt)
		combo.DeleteString(combo.GetCount()-1);
}

void CBaseBar::UpdateBaseBackBuffer(LPCTSTR left,LPCTSTR right)
{
	if(left!=NULL&&*left!=0)
		UpdateComboStrings(m_comboBasePath,left,m_nBasePathBufLen);
	if(right!=NULL&&*right!=0)
		UpdateComboStrings(m_comboBasePath2,right,m_nBasePathBufLen);
	UpdateData(FALSE);
}

inline UINT AcceptPathType(const CString& path,UINT accept_type)
{
	if(!PathFileExists(path))
		return 0;
	BOOL bDir=PathIsDirectory(path);
	if((accept_type&ACC_PATH_TYPE_DIR)&&bDir)
		return ACC_PATH_TYPE_DIR;
	if((accept_type&ACC_PATH_TYPE_FILE)&&!bDir)
		return ACC_PATH_TYPE_FILE;
	return 0;
}

BOOL CBaseBar::ValidatePaths(FListLoadData& path,UINT accept_type)
{
	if(path.mask==0)
		return FALSE;
	if(path.mask&FILE_LIST_ATTRIB_MAIN)
	{
		if(path.left.IsEmpty())
			path.mask&=~FILE_LIST_ATTRIB_MAIN;
		else if(AcceptPathType(path.left,accept_type)==0)
		{
			PDXShowMessage(_T("Invalid path: \"%s\""),(LPCTSTR)path.left);
			return FALSE;
		}
	}
	if(path.mask&FILE_LIST_ATTRIB_REF)
	{
		if(path.right.IsEmpty())
			path.mask&=~FILE_LIST_ATTRIB_REF;
		else if(AcceptPathType(path.right,ACC_PATH_TYPE_FILE)==0)
		{
			PDXShowMessage(_T("Invalid path: \"%s\""),(LPCTSTR)path.right);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CBaseBar::StartListLoad(UINT mask,UINT accept_type)
{
	if(mask&FILE_LIST_ATTRIB_MAIN)
		m_strBasePath=m_strComboBasePath;
	if(mask&FILE_LIST_ATTRIB_REF)
		m_strBasePathRef=m_strComboBasePathRef;
	FListLoadData data(m_strBasePath,m_strBasePathRef,mask);
	UpdateBaseBackBuffer((mask&FILE_LIST_ATTRIB_MAIN)?m_strComboBasePath:_T(""),
		(mask&FILE_LIST_ATTRIB_REF)?m_strComboBasePathRef:_T(""));
	if(!ValidatePaths(data,accept_type))
		return FALSE;
	if(data.mask==0)
		return TRUE;
	return PDXGetWndFromID(IDW_MAIN_VIEW)->SendMessage(WM_FILE_LIST_START_LOAD,(WPARAM)&data);
}

BOOL CBaseBar::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_btnGo.LoadBitmaps(IDB_BMP_GO_N, IDB_BMP_GO_C, IDB_BMP_GO_H, IDB_BMP_GO_D);
	m_btnGo.SizeToContent();
	m_btnOpen.m_btnMain.LoadBitmaps(IDB_BMP_OPEN_N, IDB_BMP_OPEN_C, IDB_BMP_OPEN_H, IDB_BMP_OPEN_D);
	m_btnOpen.m_btnMain.SizeToContent();
	m_btnOpen.m_btnDrop.LoadBitmaps(IDB_BMP_DROP_N, IDB_BMP_DROP_C, IDB_BMP_DROP_H, IDB_BMP_DROP_D);
	m_btnOpen.m_btnDrop.SizeToContent();
	m_btnFold.LoadBitmaps(IDB_BMP_FOLD_N, IDB_BMP_FOLD_C, IDB_BMP_FOLD_H, IDB_BMP_FOLD_D);
	m_btnFold.SizeToContent();

	m_btnGo.EnableButton(FALSE);

	m_btnGo2.LoadBitmaps(IDB_BMP_GO_N, IDB_BMP_GO_C, IDB_BMP_GO_H, IDB_BMP_GO_D);
	m_btnGo2.SizeToContent();
	m_btnOpen2.m_btnMain.LoadBitmaps(IDB_BMP_OPEN_N, IDB_BMP_OPEN_C, IDB_BMP_OPEN_H, IDB_BMP_OPEN_D);
	m_btnOpen2.m_btnMain.SizeToContent();
	m_btnOpen2.m_btnDrop.LoadBitmaps(IDB_BMP_DROP_N, IDB_BMP_DROP_C, IDB_BMP_DROP_H, IDB_BMP_DROP_D);
	m_btnOpen2.m_btnDrop.SizeToContent();
	m_btnFold2.LoadBitmaps(IDB_BMP_FOLD_N, IDB_BMP_FOLD_C, IDB_BMP_FOLD_H, IDB_BMP_FOLD_D);
	m_btnFold2.SizeToContent();

	m_btnGo2.EnableButton(FALSE);

	m_btnDFold.LoadBitmaps(IDB_BMP_DFOLD_N, IDB_BMP_DFOLD_C, IDB_BMP_DFOLD_H, IDB_BMP_DFOLD_D);
	m_btnDFold.SizeToContent();

	m_menuPopup.LoadMenu(IDR_MENU_POPUP);

	m_bInited=TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CBaseBar::OnDestroy()
{
	CDialog::OnDestroy();
	m_menuPopup.DestroyMenu();
}

void CBaseBar::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialog::OnOK();
	UpdateData(TRUE);
	m_btnGo.EnableButton(FALSE);
	m_btnGo2.EnableButton(FALSE);
	StartListLoad(FILE_LIST_ATTRIB_MAIN|FILE_LIST_ATTRIB_REF,ACC_PATH_TYPE_DIR|ACC_PATH_TYPE_FILE);
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

void CBaseBar::OnExitMenuLoop(BOOL bIsTrackPopupMenu)
{
	if(bIsTrackPopupMenu)
		RestoreCtrlState();
}

void CBaseBar::OnBnClickedButtonGo()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_btnGo.EnableButton(FALSE);
	StartListLoad(FILE_LIST_ATTRIB_MAIN,ACC_PATH_TYPE_DIR|ACC_PATH_TYPE_FILE);
}


void CBaseBar::OnBnClickedButtonOpen()
{
	UpdateData(TRUE);

	BROWSEINFO bi;
	TCHAR Buffer[MAX_PATH];
	bi.hwndOwner=AfxGetMainWnd()->GetSafeHwnd();
	bi.pidlRoot=NULL;
	bi.pszDisplayName=Buffer;
	bi.lpszTitle=_T("Select folder to count");
	bi.ulFlags=BIF_EDITBOX;
	bi.lpfn=NULL;
	bi.lParam=0;
	bi.iImage=IDR_MAINFRAME;

	LPITEMIDLIST pIDList=SHBrowseForFolder(&bi);
	RestoreCtrlState();
	if(pIDList==NULL)
		return;

	SHGetPathFromIDList(pIDList,Buffer);
	CString strPath=Buffer;

	IMalloc * imalloc=0;
	if(FAILED(SHGetMalloc(&imalloc)))
	{
		MessageBox(_T("Init IMalloc failed"));
		return;
	}
	imalloc->Free(pIDList);
	imalloc->Release();

	if(strPath.IsEmpty())
		return;

	m_strComboBasePath=strPath;
	m_btnGo.EnableButton(FALSE);
	StartListLoad(FILE_LIST_ATTRIB_MAIN,ACC_PATH_TYPE_DIR);
}


void CBaseBar::OnBnClickedButtonDrop()
{
	// TODO: Add your control notification handler code here
	CMenu *popup;
	CRect rc;
	m_btnOpen.GetWindowRect(&rc);
	popup=m_menuPopup.GetSubMenu(0);
	popup->TrackPopupMenu(TPM_RIGHTALIGN|TPM_TOPALIGN,rc.right,rc.bottom,this);
}


void CBaseBar::OnBnClickedButtonFold()
{
	// TODO: Add your control notification handler code here
}


void CBaseBar::OnBnClickedButtonGo2()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	m_btnGo2.EnableButton(FALSE);
	StartListLoad(FILE_LIST_ATTRIB_REF,ACC_PATH_TYPE_DIR|ACC_PATH_TYPE_FILE);
}


void CBaseBar::OnBnClickedButtonOpen2()
{
	// TODO: Add your control notification handler code here
}


void CBaseBar::OnBnClickedButtonDrop2()
{
	// TODO: Add your control notification handler code here
	CMenu *popup;
	CRect rc;
	m_btnOpen2.GetWindowRect(&rc);
	popup=m_menuPopup.GetSubMenu(1);
	popup->TrackPopupMenu(TPM_RIGHTALIGN|TPM_TOPALIGN,rc.right,rc.bottom,this);
}


void CBaseBar::OnBnClickedButtonFold2()
{
	// TODO: Add your control notification handler code here
}


void CBaseBar::OnBnClickedButtonDfold()
{
	// TODO: Add your control notification handler code here
}


void CBaseBar::OnCbnSelchangeComboBasePath()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	int sel=m_comboBasePath.GetCurSel();
	if(sel>=0)
		m_comboBasePath.GetLBText(sel,m_strComboBasePath);
	m_btnGo.EnableButton(FALSE);
	StartListLoad(FILE_LIST_ATTRIB_MAIN,ACC_PATH_TYPE_DIR|ACC_PATH_TYPE_FILE);
}


void CBaseBar::OnCbnSelchangeComboBasePath2()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	int sel=m_comboBasePath2.GetCurSel();
	if(sel>=0)
		m_comboBasePath2.GetLBText(sel,m_strComboBasePathRef);
	m_btnGo2.EnableButton(FALSE);
	StartListLoad(FILE_LIST_ATTRIB_REF,ACC_PATH_TYPE_DIR|ACC_PATH_TYPE_FILE);
}


void CBaseBar::OnCmdMenuOpenDir()
{
	// TODO: Add your command handler code here
	RestoreCtrlState();
	OnBnClickedButtonOpen();
}


void CBaseBar::OnCmdMenuImpFile()
{
	// TODO: Add your command handler code here
	RestoreCtrlState();
	UpdateData(TRUE);
	CString strImpFile=GetHandleFileName(m_strComboBasePath);
	if(strImpFile.IsEmpty())
		return;
	m_strComboBasePath=strImpFile;
	m_btnGo.EnableButton(FALSE);
	StartListLoad(FILE_LIST_ATTRIB_MAIN,ACC_PATH_TYPE_FILE);
}


void CBaseBar::OnCmdMenuSelectRec()
{
	// TODO: Add your command handler code here
	RestoreCtrlState();
	OnBnClickedButtonOpen2();
}


void CBaseBar::OnCmdMenuImpFileRef()
{
	// TODO: Add your command handler code here
	RestoreCtrlState();
	UpdateData(TRUE);
	CString strImpFile=GetHandleFileName(m_strComboBasePathRef);
	if(strImpFile.IsEmpty())
		return;
	m_strComboBasePathRef=strImpFile;
	m_btnGo2.EnableButton(FALSE);
	StartListLoad(FILE_LIST_ATTRIB_REF,ACC_PATH_TYPE_DIR|ACC_PATH_TYPE_FILE);
}


void ExportRecFile(CBaseBar* basebar,int side)
{
	int ret=0;
	fail_goto(ret,0,sys_mkdir((char*)CProgramData::GetExportDirPath().c_str()),fail);
	{
		if(!SendMessageToIDWnd(IDW_MAIN_VIEW,WM_LIST_FILE_VALID,(WPARAM)side))
			return;
		CString strExpFile=basebar->GetHandleFileName(a2tstr(CProgramData::GetExportFilePath()),TRUE);
		string lfile=t2astr(strExpFile);
		FListExportData data;
		if(strExpFile.IsEmpty())
			return;
		data.lfile=lfile;
		data.efile=CProgramData::GetErrListFilePath(lfile);
		data.side=side;
		SendMessageToIDWnd(IDW_MAIN_VIEW,WM_EXPORT_LIST_FILE,(WPARAM)&data);
	}
	return;
fail:
	PDXShowMessage(_T("Create export directory failed: %s"),a2t(get_error_desc(ret)));
}
void CBaseBar::OnCmdMenuExpRecFile()
{
	// TODO: Add your command handler code here
	ExportRecFile(this,-1);
}


void CBaseBar::OnCmdMenuExpRecFileRef()
{
	// TODO: Add your command handler code here
	ExportRecFile(this,1);
}
