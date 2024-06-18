#include "pch.h"
#include "MyToolBar.h"
struct CToolBarData
{
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	//WORD aItems[wItemCount]

	WORD* items()
		{ return (WORD*)(this+1); }
};
CMyToolBar::CMyToolBar():CToolBar()
{
	m_bDisableIfNoHandler=false;
}
BOOL CMyToolBar::LoadToolBar(LPCTSTR lpszResourceName,const CString& strInfo)
{
	ASSERT_VALID(this);
	ASSERT(lpszResourceName != NULL);

	// determine location of the bitmap in resource fork
	HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_TOOLBAR);
	HRSRC hRsrc = ::FindResource(hInst, lpszResourceName, RT_TOOLBAR);
	if (hRsrc == NULL)
		return FALSE;

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)
		return FALSE;

	CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
	if (pData == NULL)
		return FALSE;
	ASSERT(pData->wVersion == 1);

	CImageList image;
	CBitmap bmp;
	BITMAP bm;

	UINT* pItems = new UINT[pData->wItemCount];
	for (int i = 0; i < pData->wItemCount; i++)
		pItems[i] = pData->items()[i];
	BOOL bResult = SetButtons(pItems, pData->wItemCount);
	delete[] pItems;

	if(!bResult)
		goto end;

	// set new sizes of the buttons
	{
		CSize sizeImage(pData->wWidth, pData->wHeight);
		CSize sizeButton(pData->wWidth + 7, pData->wHeight + 7);
		SetSizes(sizeButton, sizeImage);
	}

	{
		//SetButtonInfo();
	}

	// load bitmap now that sizes are known by the toolbar control
	bResult=bmp.LoadBitmap(lpszResourceName);
	if(!bResult)
		goto end;
	bmp.GetBitmap(&bm);
	image.Create(pData->wWidth, bm.bmHeight, ILC_MASK | ILC_COLOR24, 1, 1);
	image.Add(&bmp,RGB(255,255,255));
	GetToolBarCtrl().SetImageList(&image);
	image.Detach();
	bmp.Detach();

end:
	UnlockResource(hGlobal);
	FreeResource(hGlobal);

	return bResult;
}


void CMyToolBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHandler)
{
	// TODO: Add your specialized code here and/or call the base class
	CToolBar::OnUpdateCmdUI(pTarget,m_bDisableIfNoHandler);
}
