#include "pch.h"
#include "MyToolBar.h"
#include <vector>
using namespace std;
#define SEPARATOR_THICKNESS 2
#define PLACEHOLDER_ID      1
#define TOOLBAR_HORZ_PAD    5
#define TOOLBAR_VERT_PAD    5
LPCTSTR MTB_TAGS[]={_T("NULL"),_T("CHECK"),_T("GROUP"),_T("DROP"),_T("DROPW")};
enum
{
	status_null=0,
	status_check,
	status_group,
	status_drop,
	status_dropw,
	status_num,
};
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
CMyToolBar::ItemIterator::ItemIterator(CMyToolBar* host,CRect* prcWnd)
	:m_bVert(host->IsVertical())
	,m_szBtn(host->m_szBtnOrg)
	,m_szImg(host->m_szImgOrg)
	,m_nDropWidth(host->m_nDropWidth)
	,m_nCnt(host->GetButtonCount())
	,m_pData(host->m_pData)
	,m_idx(0)
	,m_iNext(0)
{
	if(prcWnd!=NULL)
		m_rcWnd.CopyRect(prcWnd);
	else
		host->GetWindowRect(&m_rcWnd);
	host->CalcInsideRect(m_rcWnd,!m_bVert);
	m_rcWnd.OffsetRect(-m_rcWnd.TopLeft());

	m_nExBtnWidthT=CProgramData::GetRealPixelsX(m_szBtn.cx+m_nDropWidth);
	m_nBtnWidthT=CProgramData::GetRealPixelsX(m_szBtn.cx);
	m_nBtnHeightT=CProgramData::GetRealPixelsY(m_szBtn.cy);

	m_nExImgWidthT=CProgramData::GetRealPixelsX(m_szImg.cx+m_nDropWidth);
	m_nImgWidthT=CProgramData::GetRealPixelsX(m_szImg.cx);
	m_nImgHeightT=CProgramData::GetRealPixelsY(m_szImg.cy);

	m_rcBtnOffset=CRect(0,0,m_nBtnWidthT,m_nBtnHeightT);
	m_rcImgOffset=CRect(CPoint(CProgramData::GetRealPixelsX((m_szBtn.cx-m_szImg.cx)/2),
		CProgramData::GetRealPixelsY((m_szBtn.cy-m_szImg.cy)/2)),CSize(m_nImgWidthT,m_nImgHeightT));

	m_rcSep=CRect(0,m_rcImgOffset.top,SEPARATOR_THICKNESS,m_rcImgOffset.bottom);
	m_rcWrapSep=CRect(CProgramData::GetRealPixelsX((m_szBtn.cx-m_szImg.cx)/2),0,
		m_rcWnd.Width()-CProgramData::GetRealPixelsX((m_szBtn.cx-m_szImg.cx)/2),
		SEPARATOR_THICKNESS);

	m_rcImgSrc=CRect(0,0,0,m_szImg.cy);
	m_ptCur=m_rcWnd.TopLeft();

	Iterate();
}
void CMyToolBar::ItemIterator::Iterate()
{
	m_idx=m_iNext;
	if(m_idx>=m_nCnt)
		return;
	ASSERT(!(m_pData[m_idx].style&MTB_STYLE_PLACEHOLDER));
	BOOL bWrap;
	BOOL bSep=m_pData[m_idx].nID==0&&
		!(m_pData[m_idx].style&MTB_STYLE_CUSTOM);
	BOOL bDrop=(m_pData[m_idx].style&MTB_STYLE_DROPBTN);
	int idx=m_idx;
	if(m_idx+1<m_nCnt)
	{
		if(m_pData[m_idx+1].style&MTB_STYLE_PLACEHOLDER)
			m_iNext++,idx++;
		else if(m_pData[m_idx+1].nID==0)
			idx++;
	}
	bWrap=m_pData[idx].style&MTB_STYLE_WRAP;
	m_iNext++;
	CalcItemRect(bSep,bWrap,bDrop);
	if(m_iNext>=m_nCnt)
		return;
	NextItem(bSep,bWrap,bDrop);
}
void CMyToolBar::ItemIterator::CalcItemRect(BOOL bSep, BOOL bWrap, BOOL bDrop)
{
	m_rcBtn=(bSep?(bWrap?m_rcWrapSep:m_rcSep):m_rcBtnOffset)+m_ptCur;
	m_rcImg=m_rcImgOffset+m_ptCur;
	m_rcImgSrc.left=m_rcImgSrc.right;
	m_rcImgSrc.right+=(bSep?0:m_szImg.cx)+(bDrop?m_nDropWidth:0);
	if(bDrop)
	{
		m_rcDrop=m_rcBtn;
		m_rcDrop.left=m_rcImg.right;
		m_rcDrop.right=m_rcBtn.left+m_nExBtnWidthT;
		m_rcBtn.right=m_rcImg.right;
		m_rcImg.right=m_rcImg.left+m_nExImgWidthT;
	}
}
void CMyToolBar::ItemIterator::NextItem(BOOL bSep, BOOL bWrap, BOOL bDrop)
{
	if(bWrap)
	{
		m_ptCur.x=m_rcWnd.left;
		if(bSep)
			m_ptCur.y+=SEPARATOR_THICKNESS;
		else
			m_ptCur.y+=m_nBtnHeightT;
	}
	else if(bSep)
		m_ptCur.x+=SEPARATOR_THICKNESS;
	else if(bDrop)
		m_ptCur.x+=m_nExBtnWidthT;
	else
		m_ptCur.x+=m_nBtnWidthT;
}
CMyToolBar::ItemIterator::operator bool()
{
	return m_idx<m_nCnt;
}
void CMyToolBar::ItemIterator::operator++(int)
{
	Iterate();
}

CPoint CMyToolBar::s_ptBarTileOrg(-9,0);
int CMyToolBar::s_nBarRowHeight=0;

BOOL CMyToolBar::ParseConfigString(LPCTSTR strInfo,WORD* pID,int cnt,int& outcnt)
{
	UINT status=status_null;
	TCHAR buf[50];
	MyToolBarData *grpfirst=NULL,*grplast=NULL;
	vector<MyToolBarData> vData;
	MyToolBarData padD;
	memset(&padD,0,sizeof(MyToolBarData));
	int cntfinal=cnt;
	vData.reserve(2*cnt);
	vData.resize(cnt,padD);
	for(int i=0;i<cnt;i++)
		vData[i].nID=pID[i];
	vector<UINT> off_table;
	off_table.resize(cnt,0);
	for(LPCTSTR pstr=strInfo,end=strInfo;;pstr=end)
	{
		for(;*end==' ';end++);
		pstr=end;
		if(*pstr==0)
		{
			if(grplast!=NULL)
			{
				vData[(UINT*)grplast-off_table.data()].pGrpNext=grpfirst;
				grpfirst=grplast=NULL;
			}
			break;
		}
		for(;*end!=0&&*end!=' ';end++);
		if(end-pstr>49)
			return FALSE;
		memcpy(buf,pstr,(end-pstr)*sizeof(TCHAR));
		buf[end-pstr]=0;
		bool bTag=false;
		for(int i=0;i<status_num;i++)
		{
			if(_tcscmp(buf,MTB_TAGS[i])==0)
			{
				status=i;
				bTag=true;
				break;
			}
		}
		if(bTag)
		{
			if(grplast!=NULL)
			{
				vData[(UINT*)grplast-off_table.data()].pGrpNext=grpfirst;
				grpfirst=grplast=NULL;
			}
			continue;
		}
		bool bSet=false;
		LPCTSTR pstridx=buf;
		if(pstridx[0]==_T('*'))
		{
			bSet=true;
			pstridx++;
		}
		int index=-1;
		_stscanf_s(pstridx,_T("%d"),&index);
		ASSERT(status==status_dropw||(index>=0&&index<cnt));
		if(status!=status_dropw&&(index<0||index>=cnt))
			return FALSE;
		switch(status)
		{
		case status_check:
			vData[index].style|=MTB_STYLE_CHECK;
			if(bSet)
				vData[index].checked=TRUE;
			break;
		case status_group:
			vData[index].style|=MTB_STYLE_GROUPBTN;
			if(bSet)
				vData[index].checked=TRUE;
			ASSERT(vData[index].pGrpNext==NULL);
			if(vData[index].pGrpNext!=NULL)
				return FALSE;
			if(grpfirst==NULL)
				grpfirst=grplast=(MyToolBarData*)&off_table[index];
			else
			{
				ASSERT(grplast!=NULL);
				vData[(UINT*)grplast-off_table.data()].pGrpNext=
					(MyToolBarData*)&off_table[index];
				grplast=(MyToolBarData*)&off_table[index];
			}
			break;
		case status_drop:
			vData[index].style|=MTB_STYLE_DROPBTN;
			break;
		case status_dropw:
			m_nDropWidth=index;
			break;
		}
	}
	bool bcross=false;
	memset(&padD,0,sizeof(MyToolBarData));
	padD.nID=0;
	padD.style=MTB_STYLE_PLACEHOLDER;
	for(int i=0;i<cntfinal;i++)
	{
		if(i>0)
		{
			if(bcross)
				off_table[i+cnt-cntfinal]=off_table[i-1+cnt-cntfinal]+1;
			else
				off_table[i+cnt-cntfinal]=off_table[i-1+cnt-cntfinal];
		}
		bcross=false;
		if((vData[i].style&MTB_STYLE_DROPBTN) &&
			i+1<cntfinal && vData[i+1].nID!=0)
		{
			vData.insert(vData.begin()+i+1,padD);
			cntfinal++,i++;
			bcross=true;
		}
	}
	m_pData=new MyToolBarData[cntfinal];
	memcpy(m_pData,vData.data(),cntfinal*sizeof(MyToolBarData));
	for(int i=0;i<cntfinal;i++)
	{
		m_pData[i].pGrpNext=(m_pData[i].pGrpNext==NULL?NULL:
			&m_pData[*((UINT*)m_pData[i].pGrpNext)
			+(((UINT*)m_pData[i].pGrpNext)-off_table.data())]);
	}
	outcnt=cntfinal;
	return TRUE;
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

	UINT* pIDs;
	int cntfinal=0;
	BOOL bResult=ParseConfigString(strInfo,pData->items(),pData->wItemCount,cntfinal);
	if(!bResult)
		goto end;
	pIDs=new UINT[cntfinal];
	for(int i=0;i<cntfinal;i++)
		pIDs[i]=m_pData[i].nID;
	bResult=SetButtons(pIDs,cntfinal);
	delete[] pIDs;
	if(!bResult)
		goto end;

	// set new sizes of the buttons
	CalcSize(pData);

	// load bitmap now that sizes are known by the toolbar control
	bResult=m_bmpButton.LoadBitmap(lpszResourceName);

end:
	UnlockResource(hGlobal);
	FreeResource(hGlobal);

	return bResult;
}

void CMyToolBar::InitialDock(CFrameWnd* frame,BOOL bNewRow)
{
	static BOOL bFirstCall=TRUE;
	if(bFirstCall)
	{
		bFirstCall=FALSE;
		s_ptBarTileOrg=CProgramData::GetRealPoint(s_ptBarTileOrg);
	}
	CRect rect,rcBar;
	CSize size=GetBarSize()+CSize(TOOLBAR_HORZ_PAD,TOOLBAR_VERT_PAD);
	frame->GetClientRect(rect);
	rcBar=CRect(s_ptBarTileOrg,size);
	if(rcBar.right>rect.right||bNewRow)
	{
		s_ptBarTileOrg.x=0;
		s_ptBarTileOrg.y+=s_nBarRowHeight;
		s_nBarRowHeight=size.cy;
		rcBar=CRect(s_ptBarTileOrg,size);
	}
	else
	{
		s_ptBarTileOrg.x+=size.cx;
		s_nBarRowHeight=max(s_nBarRowHeight,size.cy);
	}
	ClientToScreen(&rcBar);
	frame->DockControlBar(this,(UINT)0,&rcBar);
}

void CMyToolBar::ConfigButtons(int nLength, DWORD dwMode)
{
	BOOL bDynamic=FALSE,bMRU=FALSE,
		bHorzDock=FALSE,bVertDock=FALSE,
		bHorz=FALSE,bVert=FALSE;

	if ((nLength == -1) && !(dwMode & LM_MRUWIDTH) && !(dwMode & LM_COMMIT) &&
		((dwMode & LM_HORZDOCK) || (dwMode & LM_VERTDOCK)))
	{
		bHorz = (dwMode & LM_HORZDOCK);
	}
	else
	{
		bMRU = (dwMode & LM_MRUWIDTH);
		bHorzDock = (dwMode & LM_HORZDOCK);
		bVertDock = (dwMode & LM_VERTDOCK);
		bHorz = (dwMode & LM_HORZ);
		bVert = (dwMode & LM_LENGTHY);
	}

	bDynamic = m_dwStyle & CBRS_SIZE_DYNAMIC;

	if (!(m_dwStyle & CBRS_SIZE_FIXED))
	{
		if (bDynamic && bMRU)
			ConfigButton(m_nMRUWidth);
		else if (bDynamic && bHorzDock)
			ConfigButton(32767);
		else if (bDynamic && bVertDock)
			ConfigButton(0);
		else if (bDynamic && (nLength != -1))
		{
			CRect rect; rect.SetRectEmpty();
			CalcInsideRect(rect, bHorz);
			int nLen = nLength + (bVert ? rect.Height() : rect.Width());

			ConfigButton(nLen, bVert);
		}
		else if (bDynamic && (m_dwStyle & CBRS_FLOATING))
			ConfigButton(m_nMRUWidth);
		else
			ConfigButton(bHorz ? 32767 : 0);
	}
	UpdateButtons();
}

void CMyToolBar::ConfigButton(int nLength,BOOL bVert)
{
	int nCount = GetButtonCount();
	TBBUTTON* pData = new TBBUTTON[nCount];
	ASSERT(pData != NULL && nCount > 0);
	memset(pData,0,nCount*sizeof(TBBUTTON));
	ItemIterator it(this);
	for(int i=0;i<nCount;i++)
	{
		pData[i].idCommand=m_pData[i].nID;
		if(m_pData[i].style&(MTB_STYLE_DROPBTN|MTB_STYLE_PLACEHOLDER)||
			m_pData[i].nID==0)
			pData[i].fsStyle=TBSTYLE_SEP;
		pData[i].iBitmap=(m_pData[i].style&MTB_STYLE_DROPBTN?it.m_nExBtnWidthT:
			(m_pData[i].style&MTB_STYLE_PLACEHOLDER?1:
			(m_pData[i].nID==0?SEPARATOR_THICKNESS:0)));
	}

	if (!bVert)
	{
		int nMin, nMax, nTarget, nCurrent, nMid;

		// Wrap ToolBar as specified
		nMax = nLength;
		nTarget = ArrangeButtons(pData, nCount, nMax);

		// Wrap ToolBar vertically
		nMin = 0;
		nCurrent = ArrangeButtons(pData, nCount, nMin);

		if (nCurrent != nTarget)
		{
			while (nMin < nMax)
			{
				nMid = (nMin + nMax) / 2;
				nCurrent = ArrangeButtons(pData, nCount, nMid);

				if (nCurrent == nTarget)
					nMax = nMid;
				else
				{
					if (nMin == nMid)
					{
						ArrangeButtons(pData, nCount, nMax);
						break;
					}
					nMin = nMid;
				}
			}
		}
		CSize size = CToolBar::CalcSize(pData, nCount);
		ArrangeButtons(pData, nCount, size.cx);
	}
	else
	{
		CSize sizeMax, sizeMin, sizeMid;

		// Wrap ToolBar vertically
		ArrangeButtons(pData, nCount, 0);
		sizeMin = CToolBar::CalcSize(pData, nCount);

		// Wrap ToolBar horizontally
		ArrangeButtons(pData, nCount, 32767);
		sizeMax = CToolBar::CalcSize(pData, nCount);

		while (sizeMin.cx < sizeMax.cx)
		{
			sizeMid.cx = (sizeMin.cx + sizeMax.cx) / 2;
			ArrangeButtons(pData, nCount, sizeMid.cx);
			sizeMid = CToolBar::CalcSize(pData, nCount);

			if (nLength < sizeMid.cy)
			{
				if (sizeMin == sizeMid)
				{
					ArrangeButtons(pData, nCount, sizeMax.cx);
					break;
				}
				sizeMin = sizeMid;
			}
			else if (nLength > sizeMid.cy)
			{
				if (sizeMax == sizeMid)
				{
					ArrangeButtons(pData, nCount, sizeMin.cx);
					break;
				}
				sizeMax = sizeMid;
			}
			else
				break;
		}
	}
	delete[] pData;
}

int CMyToolBar::ArrangeButtons(void* lpVoid,int nCount,int len)
{
	TBBUTTON* pData = (TBBUTTON*)lpVoid;
	ASSERT(pData != NULL && nCount > 0);

	int nResult = 0;
	int x = 0;
	for (int i = 0; i < nCount; i++)
	{
		ASSERT(!(m_pData[i].style&MTB_STYLE_PLACEHOLDER));
		pData[i].fsState&=~TBSTATE_WRAP;
		m_pData[i].style&=~MTB_STYLE_WRAP;
		int iPrev=i;
		int iPad=0;
		if (i+1<nCount&&(m_pData[i+1].style&MTB_STYLE_PLACEHOLDER))
		{
			i++;
			ASSERT(m_pData[i].nID==0);
			pData[i].fsState&=~TBSTATE_WRAP;
			m_pData[i].style&=~MTB_STYLE_WRAP;
			pData[i].idCommand=PLACEHOLDER_ID;
			iPad=pData[i].iBitmap;
		}

		int dx, dxNext;
		if (pData[iPrev].fsStyle & TBSTYLE_SEP)
		{
			dx = pData[iPrev].iBitmap;
			dxNext = dx+iPad;
		}
		else
		{
			dx = m_sizeButton.cx;
			dxNext = dx;
		}

		if (x + dx > len)
		{
			BOOL bFound = FALSE;
			for (int j = i; j >= 0  &&  !(pData[j].fsState & TBSTATE_WRAP); j--)
			{
				// Find last separator that isn't hidden
				// a separator that has a command ID is not
				// a separator, but a custom control.
				if ((pData[j].fsStyle & TBSTYLE_SEP) &&
					(pData[j].idCommand == 0))
				{
					bFound = TRUE; i = j; x = 0;
					pData[j].fsState |= TBSTATE_WRAP;
					m_pData[j].style |= MTB_STYLE_WRAP;
					nResult++;
					break;
				}
			}
			if (!bFound)
			{
				for (int j = iPrev - 1; j >= 0 && !(pData[j].fsState & TBSTATE_WRAP); j--)
				{
					// Never wrap anything that is hidden,
					// or any custom controls
					if (((pData[j].fsStyle & TBSTYLE_SEP) &&
						(pData[j].idCommand != 0)))
					{
						ASSERT(pData[j].idCommand == PLACEHOLDER_ID);
						ASSERT(m_pData[j].style & MTB_STYLE_PLACEHOLDER);
						pData[j].idCommand = 0;
					}
					else
						ASSERT(!(m_pData[j].style & MTB_STYLE_CUSTOM));

					pData[j].fsState |= TBSTATE_WRAP;
					m_pData[j].style |= MTB_STYLE_WRAP;
					bFound = TRUE; i = j; x = 0;
					nResult++;
					break;
				}
				if (!bFound)
					x += dxNext;
			}
		}
		else
			x += dxNext;
	}
	return nResult + 1;
}

void CMyToolBar::UpdateButtons()
{
	for(int i=0;i<(int)GetButtonCount();i++)
	{
		TBBUTTON button;
		if(!(m_pData[i].style&MTB_STYLE_PLACEHOLDER))
			continue;
		ASSERT(i>0);
		ASSERT(m_pData[i-1].style&MTB_STYLE_CUSTOM);
		int cmd=(m_pData[i].style&MTB_STYLE_WRAP)?0:PLACEHOLDER_ID;

		VERIFY(DefWindowProc(TB_GETBUTTON, i, (LPARAM)&button));
		if(button.idCommand!=cmd)
		{
			button.idCommand=cmd;
			VERIFY(DefWindowProc(TB_DELETEBUTTON,i,0));
			VERIFY(DefWindowProc(TB_INSERTBUTTON,i,(LPARAM)&button));
		}
	}
}

void CMyToolBar::CalcSize(void* lpVoid)
{
	CToolBarData* pData=(CToolBarData*)lpVoid;
	CSize sizeImage(pData->wWidth, pData->wHeight);
	CSize sizeButton(pData->wWidth + 7, pData->wHeight + 7);
	m_szImgOrg=sizeImage;
	m_szBtnOrg=sizeButton;
	CSize szBtnT=CProgramData::GetRealSize(m_szBtnOrg);
	CSize szImgT=CProgramData::GetRealSize(m_szImgOrg);
	int nDrop=CProgramData::GetRealPixelsX(m_szBtnOrg.cx+m_nDropWidth);
	for(int i=0;i<(int)GetButtonCount();i++)
	{
		BOOL bDrop=m_pData[i].style&MTB_STYLE_DROPBTN;
		BOOL bPad=m_pData[i].style&MTB_STYLE_PLACEHOLDER;
		BOOL bSep=m_pData[i].nID==0;
		ASSERT((!bPad)||bSep);
		SetButtonInfo(i,bPad?PLACEHOLDER_ID:m_pData[i].nID,
			MAKELONG((bDrop||bSep)?TBSTYLE_SEP:TBSTYLE_BUTTON,0),
			bDrop?nDrop:(bPad?1:(bSep?SEPARATOR_THICKNESS:0)));
	}
	SetSizes(szBtnT,szImgT);
}

CSize CMyToolBar::GetBarSize()
{
	if(m_pData==NULL)
		return CSize(0,0);
	ItemIterator it(this);
	CSize sz(0,it.m_nBtnHeightT);
	for(int i=0;i<(int)GetButtonCount();i++)
	{
		if(m_pData[i].style&MTB_STYLE_PLACEHOLDER)
			sz.cx+=1;
		else if(m_pData[i].nID==0)
			sz.cx+=SEPARATOR_THICKNESS;
		else if(m_pData[i].style&MTB_STYLE_DROPBTN)
			sz.cx+=it.m_nExBtnWidthT;
		else
			sz.cx+=it.m_nBtnWidthT;
	}
	return sz;
}

void CMyToolBar::PrepareForDraw()
{
	if(m_bDelayedButtonLayout)
		Layout();
}

BOOL CMyToolBar::HitTest(CPoint pt)
{
	if(!IsVertical())
		return pt.x <= 0;
	else
		return pt.y <= 0;
}

BOOL CMyToolBar::LButtonDown(UINT nFlags, CPoint point)
{
	// only start dragging if clicked in "void" space
	if (m_pDockBar != NULL && HitTest(point))
	{
		// start the drag
		ASSERT(m_pDockContext != NULL);
		ClientToScreen(&point);
		m_pDockContext->StartDrag(point);
		return TRUE;
	}
	return FALSE;
}
