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
protected:
	CSize GetButtonSize(){return m_sizeButton;}
	CSize GetImageSize(){return m_sizeImage;}
private:
	bool m_bDisableIfNoHandler;
	CBitmap m_bmpButton;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
};

