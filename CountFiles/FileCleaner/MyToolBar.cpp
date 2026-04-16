#include "pch.h"
#include "MyToolBar.h"
#define SAFE_DELETE(ptr) \
	if(ptr!=NULL) \
	{ \
		delete ptr; \
		ptr=NULL; \
	}
#define SAFE_DELETE_ARRAY(ptr) \
	if(ptr!=NULL) \
	{ \
		delete[] ptr; \
		ptr=NULL; \
	}

CMyToolBar::CMyToolBar():CToolBar()
{
	m_pData=NULL;
	m_pIndex=NULL;
	m_nBtnCnt=0;
	m_nBtnCntOrg=0;
	m_nDropWidth=0;
	m_iGrab=-1;
}

CMyToolBar::~CMyToolBar()
{
	if(m_pData!=NULL)
	{
		for(int i=0;i<m_nBtnCnt;i++)
		{
			SAFE_DELETE_ARRAY(m_pData[i].m_pIcons);
			if(m_pData[i].m_pAnims!=NULL)
			{
				for(int j=0;j<m_pData[i].menucnt;j++)
				{
					SAFE_DELETE_ARRAY(m_pData[i].m_pAnims[j].m_pSegs);
				}
			}
			SAFE_DELETE_ARRAY(m_pData[i].m_pAnims);
		}
	}
	SAFE_DELETE_ARRAY(m_pData);
	SAFE_DELETE_ARRAY(m_pIndex);
}

BOOL CMyToolBar::MyCreate(UINT nID,UINT nIDConf,UINT bmp,UINT bmpBack,CFrameWnd* pParentFrm,BOOL bNextRow)
{
	if (!CreateEx(pParentFrm, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP |
		CBRS_BORDER_ANY | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
		CRect(0,0,0,0), nID))
		return FALSE;

	if(!LoadToolBar(nID,nIDConf,bmp,bmpBack))
		return FALSE;

	EnableDocking(CBRS_ALIGN_ANY);
	pParentFrm->EnableDocking(CBRS_ALIGN_ANY);
	InitialDock(pParentFrm,bNextRow);

	return TRUE;
}

CSize CMyToolBar::CalcDynamicLayout(int nLength,DWORD dwMode)
{
	ConfigButtons(nLength,dwMode);
	return CToolBar::CalcDynamicLayout(nLength,dwMode);
}


BEGIN_MESSAGE_MAP(CMyToolBar, CToolBar)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_MENUSELECT()
	ON_WM_EXITMENULOOP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()


void CMyToolBar::OnDestroy()
{
	CToolBar::OnDestroy();

	m_bmpButton.DeleteObject();
	m_bmpButtonBack.DeleteObject();
}


void CMyToolBar::OnPaint()
{
	PrepareForDraw();

	CPaintDC dc(this);
	CDCDraw canvas(this,&dc,true);
	CDrawer drawer(&canvas);

	for_each_item(item)
	{
		DrawItem(drawer,item);
	}
}

void CMyToolBar::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	CToolBar::OnMenuSelect(nItemID, nFlags, hSysMenu);

	AfxGetMainWnd()->SendMessage(WM_SETMESSAGESTRING, (WPARAM)nItemID);
}

void CMyToolBar::OnExitMenuLoop(BOOL bIsTrackPopupMenu)
{
	if(bIsTrackPopupMenu)
	{
		RestoreBarState();
		Invalidate();
	}
}

void CMyToolBar::DrawItem(CDrawer& drawer,const ItemIterator& item)
{
	if(m_pData[item.m_idx].nID!=0)
	{
		if(!m_pData[item.m_idx].disabled)
		{
			if(m_pData[item.m_idx].style&MTB_STYLE_DROPBTN)
				DrawDropBtnBack(drawer,item);
			else
				DrawBtnBack(drawer,item);
		}
		DrawBtnImg(drawer,item);
	}
	else //Draw separator
	{
		drawer.DrawRect(&item.m_rcBtn,SEPARATOR_COLOR);
		drawer.FillRect(&item.m_rcBtn,SEPARATOR_COLOR);
	}
}

inline bool GetButtonColor(const MyToolBarData& data,COLORREF& Clr,COLORREF& eClr)
{
	if(data.disabled)
		return false;
	if(data.checked)
	{
		Clr=TOOLBAR_C_COLOR;
		eClr=TOOLBAR_C_E_COLOR;
		return true;
	}
	switch(data.state)
	{
	case eMTBHighlight:
		Clr=TOOLBAR_H_COLOR;
		eClr=TOOLBAR_H_E_COLOR;
		return true;
	case eMTBClick:
	case eMTBEXClick:
		Clr=TOOLBAR_C_COLOR;
		eClr=TOOLBAR_C_E_COLOR;
		return true;
	default:
		return false;
	}
}

void CMyToolBar::DrawBtnBack(CDrawer& drawer,const ItemIterator& item)
{
	COLORREF Clr,eClr;
	if(GetButtonColor(m_pData[item.m_idx],Clr,eClr))
	{
		CRect rcBtn=item.m_rcBtn;
		rcBtn.DeflateRect(CRect(2,2,2,2));
		drawer.FillRoundRect(&rcBtn,&CPoint(2,2),Clr);
		drawer.DrawRoundRect(&rcBtn,&CPoint(2,2),eClr);
	}
}

void CMyToolBar::DrawDropBtnBack(CDrawer& drawer,const ItemIterator& item)
{
	COLORREF Clr,eClr;
	if(GetButtonColor(m_pData[item.m_idx],Clr,eClr))
	{
		CRect rcBtn=item.m_rcBtn;
		CRect rcDrop=item.m_rcDrop,rcCombine;
		rcDrop.DeflateRect(CRect(0,2,2,2));
		rcBtn.DeflateRect(CRect(2,2,0,2));
		rcCombine.UnionRect(rcDrop,rcBtn);
		drawer.FillRoundRect(&rcCombine,&CPoint(2,2),Clr);
		drawer.DrawRoundRect(&rcCombine,&CPoint(2,2),eClr);
		drawer.DrawLine(&rcDrop.TopLeft(),&rcBtn.BottomRight(),eClr);
	}
}

void CMyToolBar::DrawBtnImg(CDrawer& drawer,const ItemIterator& item)
{
	MyToolBarData& tData=m_pData[item.m_idx];
	int shift;
	ImageIndex* iImg=tData.m_pIcons;
	if(iImg==NULL)
		shift=0;
	else if(tData.disabled)
		shift=iImg[tData.menuidx].m_idxDisabled;
	else
		shift=iImg[tData.menuidx].m_idxNormal;
	if((!tData.disabled)&&tData.animating&&tData.m_pAnims!=NULL)
	{
		AnimateData& anim=tData.m_pAnims[tData.menuidx];
		if(anim.m_pSegs!=NULL)
			shift=anim.m_pSegs[anim.m_iSeg].m_nImgIndex;
	}
	CRect rcSrc=item.m_rcImgSrc+CPoint(0,item.m_szImg.cy*shift);
	drawer.DrawBitmapScaled(&m_bmpButtonBack,&item.m_rcImg,&rcSrc,SRCAND);
	drawer.DrawBitmapScaled(&m_bmpButton,&item.m_rcImg,&rcSrc,SRCPAINT);
}

BOOL CMyToolBar::CheckButton(int idx,int op,int* oldstate)
{
	if(idx<0||idx>=(int)GetButtonCount())
		return FALSE;
	if(m_pData[idx].disabled)
		return FALSE;
	if(m_pData[idx].style&MTB_STYLE_GROUPBTN)
	{
		if(op!=eOn)
			return FALSE;
		for(MyToolBarData* p=m_pData[idx].pGrpNext;
			p!=&m_pData[idx];p=p->pGrpNext)
		{
			p->checked=FALSE;
		}
		BOOL checked=m_pData[idx].checked;
		m_pData[idx].checked=TRUE;
		if(oldstate!=NULL)
			*oldstate=(checked?eOn:eOff);
		if(!checked)
		{
			//TODO: handle check group btn
			Invalidate();
		}
		return TRUE;
	}
	else if(m_pData[idx].style&MTB_STYLE_CHECK)
	{
		BOOL checked=m_pData[idx].checked,changed=FALSE;
		if(oldstate!=NULL)
			*oldstate=(checked?eOn:eOff);
		switch(op)
		{
		case eOn:
			checked=TRUE;
			break;
		case eOff:
			checked=FALSE;
			break;
		case eInv:
			checked=!checked;
			break;
		}
		changed=(checked!=m_pData[idx].checked);
		m_pData[idx].checked=checked;
		if(changed)
		{
			//TODO: handle check btn
			Invalidate();
		}
		return TRUE;
	}
	return FALSE;
}

void CMyToolBar::RestoreBarState()
{
	if(m_iGrab>=0)
	{
		m_pData[m_iGrab].state=eMTBNormal;
		m_iGrab=-1;
	}
}

void CMyToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if(LButtonDown(nFlags,point))
	{
		RestoreBarState();
		Invalidate();
		return;
	}
	INT_PTR id;
	INT idx;
	BOOL drop;
	ItemFromPoint(point,&id,&drop,&idx);
	if(idx>=0&&idx==m_iGrab&&!m_pData[idx].disabled)
	{
		m_pData[idx].state=(drop?eMTBEXClick:eMTBClick);
		if(!drop)
			CheckButton(idx,m_pData[idx].style&MTB_STYLE_GROUPBTN?eOn:eInv);
	}
	Invalidate();
	CWnd::OnLButtonDown(nFlags,point);
}


void CMyToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	INT_PTR id;
	INT idx;
	BOOL drop;
	ItemFromPoint(point,&id,&drop,&idx);
	if(idx>=0&&!m_pData[idx].disabled)
	{
		ASSERT(idx==m_iGrab||m_iGrab<0);
		m_iGrab=idx;
		int state=m_pData[idx].state;
		m_pData[idx].state=eMTBHighlight;
		if(state==(drop?eMTBEXClick:eMTBClick))
		{
			//TODO: handle button click message
		}
	}
	Invalidate();
	CToolBar::OnLButtonUp(nFlags, point);
}


void CMyToolBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if(LButtonDblClk(nFlags,point))
	{
		RestoreBarState();
		Invalidate();
		return;
	}
	CWnd::OnLButtonDblClk(nFlags, point);
}


void CMyToolBar::OnMouseMove(UINT nFlags, CPoint point)
{
	INT_PTR id;
	INT idx;
	BOOL drop;
	ItemFromPoint(point,&id,&drop,&idx);
	if(idx<0||(m_iGrab>=0&&idx!=m_iGrab))
		RestoreBarState();
	if(idx>=0&&idx!=m_iGrab&&(!m_pData[idx].disabled)&&!(nFlags&MK_LBUTTON))
	{
		m_iGrab=idx;
		m_pData[idx].state=eMTBHighlight;
	}
	PDXSetMouseEvent(m_hWnd,TME_LEAVE);
	Invalidate();
	CToolBar::OnMouseMove(nFlags, point);
}


void CMyToolBar::OnMouseLeave()
{
	RestoreBarState();
	Invalidate();
	CToolBar::OnMouseLeave();
}
