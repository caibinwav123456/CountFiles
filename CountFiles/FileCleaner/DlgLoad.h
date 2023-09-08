#pragma once


// CDlgLoad dialog
#include "GenFileList.h"
class CDlgLoad;
struct FileObject
{
	string cnt_path;
	string file;
	string err_file;
	void* hFile;
	void* hFileErr;
	CDlgLoad* dlg;
	BOOL user_canceled;
	int ret;
};

class CDlgLoad : public CDialog
{
	DECLARE_DYNAMIC(CDlgLoad)

public:
	CDlgLoad(CWnd* pParent,const CString& path);   // standard constructor
	virtual ~CDlgLoad();

public:
	string GetCacheFilePath();
	string GetCacheErrFilePath();
	void UpdateProgress(const string& strPathProc);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_LOAD };
#endif
	struct
	{
		intf_cntfile* callback;
		FileObject* obj;
	}m_loadingObject;

private:
	const string m_strCachePath;
	const string m_strCacheFileName;
	const string m_strCFileExt;
	const string m_strCFileErrExt;
	CString m_strBasePath;
	CString m_strProgressBuf;

	void* m_hThreadLoadFile;
	CRITICAL_SECTION m_cs;

	string GetCFilePathRoot();
	void SplitPathDisplay();

	BOOL StartLoadingThread();
	void ShowMessage(LPCTSTR format,...);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	CString m_strPathLoading;
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnLoadingComplete(WPARAM wParam,LPARAM lParam);
	virtual void OnOK();
	virtual void OnCancel();
};
