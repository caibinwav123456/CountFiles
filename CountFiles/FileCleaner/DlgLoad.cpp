// DlgLoad.cpp : implementation file
//

#include "pch.h"
#include "FileCleaner.h"
#include "DlgLoad.h"
#include <afxdialogex.h>
#include <assert.h>
#define safe_fclose(hfile) \
	if(VALID(hfile)) \
	{ \
		sys_fclose(hfile); \
		hfile=NULL; \
	}
#define safe_delete_ptr(ptr) \
	if(ptr!=NULL) \
	{ \
		delete ptr; \
		ptr=NULL; \
	}
#define safe_delete_obj(_obj) \
	safe_delete_ptr(_obj.obj); \
	safe_delete_ptr(_obj.callback);

#define IDEVENT_TIMER_PROG 100

// CDlgLoad dialog

IMPLEMENT_DYNAMIC(CDlgLoad, CDialog)

CDlgLoad::CDlgLoad(CWnd* pParent,const string& path,
	const string& lfile,const string& efile)
	: CDialog(IDD_DIALOG_LOAD, pParent)
	, m_strPathLoading(_T(""))
	, m_strBasePath(path)
	, m_strListFile(lfile)
	, m_strErrFile(efile)
	, m_hThreadLoadFile(NULL)
{
	m_loadingObject.callback=NULL;
	m_loadingObject.obj=NULL;
	memset(&m_cs,0,sizeof(m_cs));
}

CDlgLoad::~CDlgLoad()
{
}

void CDlgLoad::SplitPathDisplay()
{
	CRect rcWnd;
	m_StaticPath.GetWindowRect(&rcWnd);
	int width=rcWnd.Width();
	CWindowDC dc(&m_StaticPath);
	for(uint i=0,len=m_strPathLoading.GetLength();i<len;i++,len++)
	{
		uint acc=0;
		for(uint j=64;j>0;j>>=1)
		{
			uint na=acc|j;
			uint next=i+na;
			if(next>len)
				next=len;
			if(dc.GetTextExtent(m_strPathLoading.Mid(i,next-i)).cx<=width)
			{
				if(next==len)
					return;
				acc=na;
			}
		}
		assert(acc>0);
		i+=acc;
		if(i<len)
			m_strPathLoading.Insert(i,_T("\n"));
	}
}

void CDlgLoad::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_PATH, m_strPathLoading);
	DDX_Control(pDX, IDC_STATIC_PATH, m_StaticPath);
}


BEGIN_MESSAGE_MAP(CDlgLoad, CDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_LOADING_COMPLETE,OnLoadingComplete)
END_MESSAGE_MAP()


// CDlgLoad message handlers


BOOL CDlgLoad::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	sys_fdelete((char*)m_strListFile.c_str());
	sys_fdelete((char*)m_strErrFile.c_str());

	InitializeCriticalSection(&m_cs);
	if(!StartLoadingThread())
	{
		DeleteCriticalSection(&m_cs);
		goto exitdlg;
	}
	SetTimer(IDEVENT_TIMER_PROG,100,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
exitdlg:
	CDialog::OnCancel();
	return TRUE;
}

static int cb_wr_file_info(byte* buf,uint buflen,void* param)
{
	FileObject* obj=(FileObject*)param;
	return sys_fwrite(obj->hFile,buf,buflen,NULL);
}
static int cb_wr_file_err(byte* buf,uint buflen,void* param)
{
	FileObject* obj=(FileObject*)param;
	if(obj->hFileErr==NULL)
	{
		obj->hFileErr=sys_fopen((char*)obj->err_file.c_str(),FILE_WRITE|FILE_CREATE_ALWAYS);
		if(!VALID(obj->hFileErr))
			return ERR_OPEN_FILE_FAILED;
	}
	return sys_fwrite(obj->hFileErr,buf,buflen,NULL);
}
static int cb_cnt_file_prog(byte* buf,uint buflen,void* param)
{
	FileObject* obj=(FileObject*)param;
	static int interval=0;
	if(interval>=10)
	{
		obj->dlg->UpdateProgress((char*)buf);
		interval=0;
	}
	else
		interval++;
	return obj->user_canceled?-1:0;
}
void clean_write_obj(FileObject* obj,bool bdelete)
{
	safe_fclose(obj->hFile);
	safe_fclose(obj->hFileErr);
	if(bdelete)
	{
		sys_fdelete((char*)obj->file.c_str());
		sys_fdelete((char*)obj->err_file.c_str());
	}
	obj->file.clear();
	obj->err_file.clear();
}
int count_path(void* param/*const char* cnt_path,const char* listfile,const char* errfile*/)
{
	intf_cntfile* callback=(intf_cntfile*)param;
	FileObject* obj=(FileObject*)callback->param;
	obj->ret=GenFileList(obj->cnt_path.c_str(),callback);
	obj->dlg->PostMessage(WM_LOADING_COMPLETE);
	return 0;
}

BOOL CDlgLoad::StartLoadingThread()
{
	intf_cntfile* callback=new intf_cntfile;
	FileObject* obj=new FileObject;
	callback->param=obj;
	callback->cb_info=cb_wr_file_info;
	callback->cb_error=cb_wr_file_err;
	callback->cb_rec=cb_cnt_file_prog;

	obj->ret=0;
	obj->cnt_path=m_strBasePath;
	obj->file=m_strListFile;
	obj->err_file=m_strErrFile;
	obj->dlg=this;
	obj->user_canceled=false;

	m_loadingObject.callback=callback;
	m_loadingObject.obj=obj;

	obj->hFile=sys_fopen((char*)obj->file.c_str(),FILE_WRITE|FILE_CREATE_ALWAYS);
	obj->hFileErr=NULL;

	if(!VALID(obj->hFile))
	{
		PDXShowMessage(_T("\"%s\": can not open file for writing"),a2t(obj->file));
		safe_delete_obj(m_loadingObject);
		return FALSE;
	}
	m_hThreadLoadFile=sys_create_thread(count_path,callback);
	if(!VALID(m_hThreadLoadFile))
	{
		clean_write_obj(obj,true);
		safe_delete_obj(m_loadingObject);
		return FALSE;
	}

	return TRUE;
}

void CDlgLoad::UpdateProgress(const string& strPathProc)
{
	EnterCriticalSection(&m_cs);
	m_strProgressBuf=a2tstr(strPathProc);
	LeaveCriticalSection(&m_cs);
}

LRESULT CDlgLoad::OnLoadingComplete(WPARAM wParam, LPARAM lParam)
{
	sys_wait_thread(m_hThreadLoadFile);
	sys_close_thread(m_hThreadLoadFile);
	m_hThreadLoadFile=NULL;
	KillTimer(IDEVENT_TIMER_PROG);

	if(m_loadingObject.obj->user_canceled)
	{
		clean_write_obj(m_loadingObject.obj,true);
		safe_delete_obj(m_loadingObject);
		CDialog::OnCancel();
	}
	else if(m_loadingObject.obj->ret!=0)
	{
		clean_write_obj(m_loadingObject.obj,true);
		PDXShowMessage(_T("GenFileList failed: %s"),a2t(get_error_desc(m_loadingObject.obj->ret)));
		safe_delete_obj(m_loadingObject);
		CDialog::OnCancel();
	}
	else
	{
		clean_write_obj(m_loadingObject.obj,false);
		safe_delete_obj(m_loadingObject);
		OnOK();
	}
	return 0;
}

void CDlgLoad::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent==IDEVENT_TIMER_PROG)
	{
		EnterCriticalSection(&m_cs);
		m_strPathLoading=m_strProgressBuf;
		LeaveCriticalSection(&m_cs);
		SplitPathDisplay();
		UpdateData(FALSE);
	}
	CDialog::OnTimer(nIDEvent);
}


void CDlgLoad::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here
	DeleteCriticalSection(&m_cs);
}


void CDlgLoad::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialog::OnOK();
}


void CDlgLoad::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class
	m_loadingObject.obj->user_canceled=true;
}
