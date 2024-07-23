#pragma once
struct MyToolBarData
{
	UINT nID;
	UINT style;
	UINT state;
	UINT nCmdMsg;
};
class CMyToolBar :
	public CToolBar
{
public:
	CMyToolBar();
	~CMyToolBar();
	BOOL LoadToolBar(UINT nIDResource)
	{
		CString strInfo;
		strInfo.LoadString(nIDResource);
		return LoadToolBar(MAKEINTRESOURCE(nIDResource),strInfo);
	}
	BOOL LoadToolBar(LPCTSTR lpszResourceName,const CString& strInfo);
protected:
	CSize GetButtonSize(){return m_sizeButton;}
	CSize GetImageSize(){return m_sizeImage;}
	UINT GetButtonCount(){return m_nCount;}
	BOOL IsVertical(){return !!(GetStyle()&(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT));}
private:
	CBitmap m_bmpButton;
	MyToolBarData* m_pData;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
};

