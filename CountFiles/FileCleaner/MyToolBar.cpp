#include "pch.h"
#include "MyToolBar.h"
#include "DrawObject.h"
#define SEPARATOR_THICKNESS 2
#define SEPARATOR_COLOR     RGB(140,140,140)
#define TOOLBAR_HORZ_PAD    5
#define TOOLBAR_VERT_PAD    5
#define for_each_item(iter) for(ItemIterator iter(this);iter;iter++)
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
CMyToolBar::ItemIterator::ItemIterator(CMyToolBar* host,BOOL vert,CRect* prcWnd)
	:m_bVert(FALSE)
	,m_szBtn(host->m_szBtnOrg)
	,m_szImg(host->m_szImgOrg)
	,m_nDropWidth(host->m_nDropWidth)
	,m_nCnt(host->GetButtonCount())
	,m_pData(host->m_pData)
	,m_idx(0)
{
	if(prcWnd==NULL)
	{
		host->GetWindowRect(&m_rcWnd);
		m_bVert=host->IsVertical();
	}
	else
	{
		m_rcWnd.CopyRect(prcWnd);
		m_bVert=vert;
	}
	if(!m_bVert)
	{
		m_ptBtnOffset=CPoint(0,(m_rcWnd.Height()-CProgramData::GetRealPixelsY(m_szBtn.cy))/2);
		m_ptImgOffset=CPoint(CProgramData::GetRealPixelsX((m_szBtn.cx-m_szImg.cx)/2),
			(m_rcWnd.Height()-CProgramData::GetRealPixelsY(m_szImg.cy))/2);
	}
	else
	{
		m_ptBtnOffset=CPoint((m_rcWnd.Width()-CProgramData::GetRealPixelsX(m_szBtn.cx))/2,0);
		m_ptImgOffset=CPoint((m_rcWnd.Width()-CProgramData::GetRealPixelsX(m_szImg.cx))/2,
			CProgramData::GetRealPixelsY((m_szBtn.cy-m_szImg.cy)/2));
	}

	m_nExBtnWidthT=CProgramData::GetRealPixelsX(m_szBtn.cx+m_nDropWidth);
	m_nBtnWidthT=CProgramData::GetRealPixelsX(m_szBtn.cx);
	m_nBtnHeightT=CProgramData::GetRealPixelsY(m_szBtn.cy);

	m_nExImgWidthT=CProgramData::GetRealPixelsX(m_szImg.cx+m_nDropWidth);
	m_nImgWidthT=CProgramData::GetRealPixelsX(m_szImg.cx);
	m_nImgHeightT=CProgramData::GetRealPixelsY(m_szImg.cy);

	if(m_nCnt==0)
		return;
	if(m_pData[0].nID==0)
	{
		if(!m_bVert)
		{
			m_rcBtn=CRect(m_ptBtnOffset,CSize(SEPARATOR_THICKNESS,m_nBtnHeightT));
			m_rcImg=CRect(m_ptImgOffset,CSize(SEPARATOR_THICKNESS,m_nImgHeightT));
		}
		else
		{
			m_rcBtn=CRect(m_ptBtnOffset,CSize(m_nBtnWidthT,SEPARATOR_THICKNESS));
			m_rcImg=CRect(m_ptImgOffset,CSize(m_nImgWidthT,SEPARATOR_THICKNESS));
		}
		m_rcImgSrc=CRect(0,0,0,m_szImg.cy);
		return;
	}
	m_rcBtn=CRect(m_ptBtnOffset,CSize(m_nBtnWidthT,m_nBtnHeightT));
	m_rcImg=CRect(m_ptImgOffset,CSize(m_nImgWidthT,m_nImgHeightT));
	m_rcImgSrc=CRect(CPoint(0,0),m_szImg);
	if(IsDropDown(0))
	{
		m_rcDrop=m_rcBtn;
		m_rcDrop.left=m_rcImg.right;
		m_rcDrop.right=m_rcBtn.left+m_nExBtnWidthT;
		m_rcBtn.right=m_rcImg.right;
		m_rcImg.right=m_rcImg.left+m_nExImgWidthT;
		m_rcImgSrc.right+=m_nDropWidth;
	}
}
CMyToolBar::ItemIterator::operator bool()
{
	return m_idx<m_nCnt;
}
void CMyToolBar::ItemIterator::operator++(int)
{
	m_idx++;
	if(m_idx>=m_nCnt)
		return;
	if(!m_bVert)
	{
		if(m_pData[m_idx-1].nID!=0)
		{
			bool bdrop=IsDropDown(m_idx-1);
			m_rcBtn.OffsetRect(bdrop?m_nExBtnWidthT:m_nBtnWidthT,0);
			m_rcImg.OffsetRect(bdrop?m_nExBtnWidthT:m_nBtnWidthT,0);
			m_rcImgSrc.OffsetRect(bdrop?m_szImg.cx+m_nDropWidth:m_szImg.cx,0);
		}
		else
		{
			m_rcBtn.OffsetRect(SEPARATOR_THICKNESS,0);
			m_rcImg.OffsetRect(SEPARATOR_THICKNESS,0);
		}
		if(m_pData[m_idx].nID==0)
		{
			m_rcBtn.right=m_rcBtn.left+SEPARATOR_THICKNESS;
			m_rcImg.right=m_rcImg.left+SEPARATOR_THICKNESS;
			m_rcImgSrc.right=m_rcImgSrc.left;
		}
	}
	else
	{
		if(m_pData[m_idx-1].nID!=0)
		{
			m_rcBtn.OffsetRect(0,m_nBtnHeightT);
			m_rcImg.OffsetRect(0,m_nBtnHeightT);
			m_rcImgSrc.OffsetRect(IsDropDown(m_idx-1)?m_szImg.cx+m_nDropWidth:m_szImg.cx,0);
		}
		else
		{
			m_rcBtn.OffsetRect(0,SEPARATOR_THICKNESS);
			m_rcImg.OffsetRect(0,SEPARATOR_THICKNESS);
		}
		if(m_pData[m_idx].nID==0)
		{
			m_rcBtn.bottom=m_rcBtn.top+SEPARATOR_THICKNESS;
			m_rcBtn.right=m_rcBtn.left+m_nBtnWidthT;
			m_rcImg.bottom=m_rcImg.top+SEPARATOR_THICKNESS;
			m_rcImg.right=m_rcImg.left+m_nImgWidthT;
			m_rcImgSrc.right=m_rcImgSrc.left;
		}
	}
	if(m_pData[m_idx].nID!=0)
	{
		if(m_bVert)
		{
			m_rcBtn.bottom=m_rcBtn.top+m_nBtnHeightT;
			m_rcImg.bottom=m_rcImg.top+m_nImgHeightT;
		}
		if(IsDropDown(m_idx))
		{
			m_rcDrop=m_rcBtn;
			m_rcBtn.right=m_rcDrop.left=m_rcImg.left+m_nImgWidthT;
			m_rcDrop.right=m_rcBtn.left+m_nExBtnWidthT;
			m_rcImg.right=m_rcImg.left+m_nExImgWidthT;
			m_rcImgSrc.right=m_rcImgSrc.left+m_szImg.cx+m_nDropWidth;
		}
		else
		{
			m_rcBtn.right=m_rcBtn.left+m_nBtnWidthT;
			m_rcImg.right=m_rcImg.left+m_nImgWidthT;
			m_rcImgSrc.right=m_rcImgSrc.left+m_szImg.cx;
		}
	}
}
CPoint CMyToolBar::s_ptBarTileOrg(0,0);
int CMyToolBar::s_nBarRowHeight=0;
CMyToolBar::CMyToolBar():CToolBar()
{
	m_pData=NULL;
	m_nDropWidth=0;
	m_nDropCnt=0;
}
CMyToolBar::~CMyToolBar()
{
	if (m_pData!=NULL)
		delete[] m_pData;
}
BOOL CMyToolBar::ParseConfigString(LPCTSTR strInfo)
{
	UINT status=status_null;
	TCHAR buf[50];
	MyToolBarData *grpfirst=NULL,*grplast=NULL;
	for(LPCTSTR pstr=strInfo,end=strInfo;*pstr!=0;pstr=end)
	{
		for(;*end==' ';end++);
		pstr=end;
		if(*pstr==0)
		{
			if(grplast!=NULL)
			{
				grplast->pGrpNext=grpfirst;
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
				grplast->pGrpNext=grpfirst;
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
		ASSERT(status==status_dropw||(index>=0&&index<(int)GetButtonCount()));
		if(status!=status_dropw&&(index<0||index>=(int)GetButtonCount()))
			return FALSE;
		switch(status)
		{
		case status_check:
			m_pData[index].style|=MTB_STYLE_CHECK;
			if(bSet)
				m_pData[index].checked=TRUE;
			break;
		case status_group:
			if(bSet)
				m_pData[index].checked=TRUE;
			ASSERT(m_pData[index].pGrpNext==NULL);
			if(m_pData[index].pGrpNext!=NULL)
				return FALSE;
			m_pData[index].style|=MTB_STYLE_GROUPBTN;
			if(grpfirst==NULL)
				grpfirst=grplast=&m_pData[index];
			else
			{
				ASSERT(grplast!=NULL);
				grplast->pGrpNext=&m_pData[index];
				grplast=&m_pData[index];
			}
			break;
		case status_drop:
			m_pData[index].style|=MTB_STYLE_DROPBTN;
			break;
		case status_dropw:
			m_nDropWidth=index;
			break;
		}
	}
	m_nDropCnt=0;
	for(int i=0;i<(int)GetButtonCount();i++)
	{
		if(m_pData[i].style&MTB_STYLE_DROPBTN)
			m_nDropCnt++;
	}
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

	UINT* pItems = new UINT[pData->wItemCount];
	for (int i = 0; i < pData->wItemCount; i++)
		pItems[i] = pData->items()[i];
	BOOL bResult = SetButtons(pItems, pData->wItemCount);
	if(!bResult)
	{
		delete[] pItems;
		goto end;
	}
	delete[] pItems;
	m_pData=new MyToolBarData[m_nCount];
	memset(m_pData,0,m_nCount*sizeof(MyToolBarData));
	for(int i=0;i<m_nCount;i++)
		m_pData[i].nID=pData->items()[i];
	bResult=ParseConfigString(strInfo);
	if(!bResult)
	{
		delete[] m_pData;
		m_pData=NULL;
		goto end;
	}

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

CSize CMyToolBar::CalcDynamicLayout(int nLength,DWORD dwMode)
{
	CSize sz=CToolBar::CalcDynamicLayout(nLength,dwMode);
	if((dwMode&(LM_VERTDOCK))&&m_nDropCnt>0)
	{
		sz.cx+=CProgramData::GetRealPixelsX(2*m_nDropWidth);
		int nDrop=CProgramData::GetRealPixelsX(m_szBtnOrg.cx+m_nDropWidth);
		sz.cy-=m_nDropCnt*(nDrop-SEPARATOR_THICKNESS);
	}
	return sz;
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
		BOOL bSep=m_pData[i].nID==0;
		SetButtonInfo(i,bDrop?0:m_pData[i].nID,
			MAKELONG((bDrop||bSep)?TBSTYLE_SEP:TBSTYLE_BUTTON,TBSTATE_ENABLED),
			bDrop?nDrop:(bSep?SEPARATOR_THICKNESS:0));
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
		if(m_pData[i].nID==0)
			sz.cx+=SEPARATOR_THICKNESS;
		else if(m_pData[i].style&MTB_STYLE_DROPBTN)
			sz.cx+=it.m_nExBtnWidthT;
		else
			sz.cx+=it.m_nBtnWidthT;
	}
	return sz;
}

BEGIN_MESSAGE_MAP(CMyToolBar, CToolBar)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


void CMyToolBar::OnPaint()
{
	if(m_bDelayedButtonLayout)
		Layout();

	CPaintDC dc(this); // device context for painting
					   // TODO: Add your message handler code here
					   // Do not call CToolBar::OnPaint() for painting messages
	CDCDraw canvas(this,&dc,true);
	CDrawer drawer(&canvas);
	for_each_item(btn)
	{
		if(m_pData[btn.m_idx].nID!=0)
			drawer.DrawBitmapScaled(&m_bmpButton,&btn.m_rcImg,&btn.m_rcImgSrc);
		else
		{
			drawer.DrawRect(&btn.m_rcImg,SEPARATOR_COLOR);
			drawer.FillRect(&btn.m_rcImg,SEPARATOR_COLOR);
		}
	}
}


BOOL CMyToolBar::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	return CToolBar::OnEraseBkgnd(pDC);
}


void CMyToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CToolBar::OnLButtonDown(nFlags, point);
}


void CMyToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CToolBar::OnLButtonUp(nFlags, point);
}


void CMyToolBar::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CToolBar::OnMouseMove(nFlags, point);
}


void CMyToolBar::OnMouseLeave()
{
	// TODO: Add your message handler code here and/or call default

	CToolBar::OnMouseLeave();
}


void CMyToolBar::OnDestroy()
{
	CToolBar::OnDestroy();

	// TODO: Add your message handler code here
	m_bmpButton.DeleteObject();
}
