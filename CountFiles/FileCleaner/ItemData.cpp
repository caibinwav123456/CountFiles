#include "pch.h"
#include "TreeListCtrl.h"
#include <assert.h>
uint TLItemDir::GetDispLength()
{
	return isopen?open_length:1;
}
void TLItemDir::update_displen(int diff)
{
	for(TLItemDir* pp=parent;pp!=NULL;pp=pp->parent)
	{
		assert(pp->open_length>0);
		pp->open_length+=diff;
	}
}
int TLItemDir::OpenDir(bool open,bool release)
{
	int ret=0;
	uint oldlen=GetDispLength();
	isopen=open;
	if(open)
	{
		if(open_length>0)
			return 0;
		if(0!=(ret=ctx->ExpandNode(dirnode,true,release)))
			goto fail;
		if(0!=(ret=construct_list()))
		{
			ctx->ExpandNode(dirnode,false,true);
			goto fail;
		}
		update_displen(GetDispLength()-oldlen);
		return 0;
	}
	else if(release)
	{
		clear();
		ret=ctx->ExpandNode(dirnode,false,true);
		update_displen(GetDispLength()-oldlen);
		return ret;
	}
	else if(0!=(ret=ctx->ExpandNode(dirnode,false)))
		goto fail;
	update_displen(GetDispLength()-oldlen);
	return 0;
fail:
	isopen=false;
	clear();
	update_displen(GetDispLength()-oldlen);
	return ret;
}
void TLItemDir::clear()
{
	open_length=0;
	dir_border=0;
	subitems.clear();
	for(int i=0;i<(int)subfiles.size();i++)
	{
		subfiles[i]->Release();
		delete subfiles[i];
	}
	subfiles.clear();
	for(int i=0;i<(int)errfiles.size();i++)
	{
		errfiles[i]->Release();
		delete errfiles[i];
	}
	errfiles.clear();
	for(int i=0;i<(int)errdirs.size();i++)
	{
		errdirs[i]->Release();
		delete errdirs[i];
	}
	errdirs.clear();
	for(int i=0;i<(int)subdirs.size();i++)
	{
		subdirs[i]->Release();
		delete subdirs[i];
	}
	subdirs.clear();
}
void TLItemDir::Release()
{
	OpenDir(false,true);
}
static inline ItStkItem* push_item_stack(ItStkItem* stk)
{
	ItStkItem* newitem=new ItStkItem(NULL);
	newitem->parentidx=0;
	newitem->next=stk;
	return newitem;
}
static inline void free_item_stack(ItStkItem* stk)
{
	while(stk!=NULL)
	{
		ItStkItem* item=stk;
		stk=stk->next;
		delete item;
	}
}
ItStkItem* TLItem::FromLineNum(int iline,int& lvl)
{
	if(iline<0)
		return NULL;
	TLItemDir* cur=dynamic_cast<TLItemDir*>(this);
	assert(cur!=NULL);
	ItStkItem* stack=push_item_stack(NULL);
	int iacc=0;
	int level=0;
	for(;;)
	{
		if(stack->parentidx>=(int)cur->subitems.size())
		{
			free_item_stack(stack);
			return NULL;
		}
		TLItem* item=cur->subitems[stack->parentidx];
		if(iacc==iline)
		{
			stack->m_pLItem=item;
			lvl=level;
			return stack;
		}
		int displen=item->GetDispLength();
		if(iacc+displen>iline)
		{
			cur=dynamic_cast<TLItemDir*>(item);
			assert(cur!=NULL);
			stack=push_item_stack(stack);
			level++;
		}
		else
		{
			iacc+=displen;
			stack->parentidx++;
		}
	}
}
int TLItem::ToLineNum(TLItem* item)
{
	return -1;
}
ListCtrlDrawIterator::ListCtrlDrawIterator(ListCtrlDrawIterator& other)
{
	memcpy(this,&other,sizeof(*this));
	other.m_pStkItem=NULL;
	other.lvl=0;
}
ListCtrlDrawIterator::~ListCtrlDrawIterator()
{
	free_item_stack(m_pStkItem);
}
ListCtrlDrawIterator::operator bool()
{
	if(end)
		return false;
	end=(m_pStkItem==NULL||!m_pList->EndOfDraw(m_iline));
	return !end;
}
void ListCtrlDrawIterator::operator++(int)
{
	if(end||m_pStkItem==NULL)
		return;
	m_iline++;
	if(m_iline<=0)
		return;
	if(m_pStkItem->m_pLItem->type==eITypeDir)
	{
		TLItemDir* dir=dynamic_cast<TLItemDir*>(m_pStkItem->m_pLItem);
		assert(dir!=NULL);
		if(dir->isopen&&!dir->subitems.empty())
		{
			m_pStkItem=push_item_stack(m_pStkItem);
			m_pStkItem->m_pLItem=dir->subitems[0];
			lvl++;
			return;
		}
	}
	for(;;)
	{
		if(m_pStkItem->parentidx+1<(int)m_pStkItem->m_pLItem->parent->subitems.size())
		{
			m_pStkItem->parentidx++;
			m_pStkItem->m_pLItem=m_pStkItem->m_pLItem->parent->subitems[m_pStkItem->parentidx];
			return;
		}
		if(lvl==0)
		{
			end=true;
			return;
		}
		lvl--;
		ItStkItem* next=m_pStkItem->next;
		delete m_pStkItem;
		m_pStkItem=next;
	}
}
void ListCtrlDrawIterator::operator--(int)
{
	if(m_pStkItem==NULL)
		return;
	if(m_iline<0)
		return;
	m_iline--;
	if(m_iline<0)
		return;
	if(end)
	{
		assert(lvl==0&&m_pStkItem->next==NULL);
		end=false;
		goto second_phase;
	}
	for(;;)
	{
		if(m_pStkItem->parentidx>0)
		{
			m_pStkItem->parentidx--;
			m_pStkItem->m_pLItem=m_pStkItem->m_pLItem->parent->subitems[m_pStkItem->parentidx];
			break;
		}
		if(lvl==0)
			return;
		lvl--;
		ItStkItem* next=m_pStkItem->next;
		delete m_pStkItem;
		m_pStkItem=next;
	}
second_phase:
	for(;;)
	{
		if(m_pStkItem->m_pLItem->type!=eITypeDir)
			break;
		TLItemDir* dir=dynamic_cast<TLItemDir*>(m_pStkItem->m_pLItem);
		assert(dir!=NULL);
		if((!dir->isopen)||dir->subitems.empty())
			break;
		m_pStkItem=push_item_stack(m_pStkItem);
		m_pStkItem->parentidx=dir->subitems.size()-1;
		m_pStkItem->m_pLItem=dir->subitems.back();
		lvl++;
	}
}
bool ListCtrlDrawIterator::operator==(const ListCtrlDrawIterator& other) const
{
	return m_iline==other.m_iline;
}
bool ListCtrlDrawIterator::operator!=(const ListCtrlDrawIterator& other) const
{
	return m_iline!=other.m_iline;
}
bool ListCtrlDrawIterator::operator>(const ListCtrlDrawIterator& other) const
{
	return m_iline>other.m_iline;
}
bool ListCtrlDrawIterator::operator<(const ListCtrlDrawIterator& other) const
{
	return m_iline<other.m_iline;
}
bool ListCtrlDrawIterator::operator>=(const ListCtrlDrawIterator& other) const
{
	return m_iline>=other.m_iline;
}
bool ListCtrlDrawIterator::operator<=(const ListCtrlDrawIterator& other) const
{
	return m_iline<=other.m_iline;
}
TLItem* ItemSelector::GetSel()
{
	return m_pItemSel;
}
bool ItemSelector::InDragRegion(int iline,bool* cancel)
{
	if(iline<0)
		return false;
	int start=m_iDragStart,end=m_iDragEnd;
	if(start<end)
		swap(start,end);
	if(iline>=start&&iline<=end)
	{
		if(cancel!=NULL)
			*cancel=m_bCancelRgn;
		return true;
	}
	return false;
}
bool ItemSelector::IsSelected(TLItem* item,int iline)
{
	if(item==NULL)
		return false;
	if(InDragRegion(iline))
		return !m_bCancelRgn;
	return item==m_pItemSel;
}
void ItemSelector::SetSel(TLItem* item)
{
	for(set<TLItem*>::iterator it=m_setSel.begin();it!=m_setSel.end();it++)
	{
		(*it)->issel=false;
	}
	m_setSel.clear();
	m_pItemSel=NULL;
	m_pItemFocus=NULL;
	if(item==NULL)
		return;
	AddSel(item);
}
void ItemSelector::AddSel(TLItem* item)
{
	if(item==NULL)
		return;
	assert(item->issel==false);
	assert(m_setSel.find(item)==m_setSel.end());
	item->issel=true;
	m_setSel.insert(item);
	m_pItemSel=item;
	m_pItemFocus=item;
}
void ItemSelector::CancelSel(TLItem* item)
{
	if(item==NULL)
		return;
	assert(item->issel==true);
	assert(m_setSel.find(item)!=m_setSel.end());
	item->issel=false;
	m_setSel.erase(item);
	m_pItemSel=item;
	m_pItemFocus=item;
}
void ItemSelector::ToggleSel(TLItem* item)
{
	if(item==NULL)
		return;
	if(item->issel)
		CancelSel(item);
	else
		AddSel(item);
}
bool ItemSelector::BeginDragSel(int iline,bool bcancel)
{
	if(iline<0||iline>=(int)m_pOwner->m_nTotalLine)
		return false;
	m_iDragStart=iline;
	return true;
}
bool ItemSelector::DragSelTo(int iline)
{
	if(iline<0||iline>=(int)m_pOwner->m_nTotalLine)
		return false;
	m_iDragEnd=iline;
	return true;
}
void ItemSelector::EndDragSel(int iline)
{
	DragSelTo(iline);

	m_iDragStart=m_iDragEnd=-1;
}
void ItemSelector::SortSelection()
{

}
