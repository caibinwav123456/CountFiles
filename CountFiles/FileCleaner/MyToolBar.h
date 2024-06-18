#pragma once
class CMyToolBar :
	public CToolBar
{
public:
	CMyToolBar();
	BOOL LoadToolBar(UINT nIDResource)
	{
		CString strInfo;
		strInfo.LoadString(nIDResource);
		return LoadToolBar(MAKEINTRESOURCE(nIDResource),strInfo);
	}
	BOOL LoadToolBar(LPCTSTR lpszResourceName,const CString& strInfo);
public:
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHandler);
private:
	bool m_bDisableIfNoHandler;
};

