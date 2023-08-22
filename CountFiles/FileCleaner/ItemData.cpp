#include "pch.h"
#include "TreeListCtrl.h"
#include <assert.h>
TLItem** TLItem::GetPeerItem(TLItem*** _this)
{
	if(parent==NULL||parent->subpairs==NULL)
		return NULL;
	TLItemPair* tuple=&parent->subpairs->map[parentidx];
	assert(!(tuple->left==NULL&&tuple->right==NULL));
	assert(tuple->left==this||tuple->right==this);
	if(tuple->left==this)
	{
		if(_this!=NULL)
			*_this=&tuple->left;
		return &tuple->right;
	}
	else //tuple->right==this
	{
		if(_this!=NULL)
			*_this=&tuple->right;
		return &tuple->left;
	}
}
void TLItemSplice::clear()
{
	if(this==NULL)
		return;
#define restore_parentidx(pair,domain,ptr) if(pair.domain!=NULL)pair.domain->parentidx=-1;
	for(int i=0;i<(int)map.size();i++)
	{
		restore_parentidx(map[i],left,this);
		restore_parentidx(map[i],right,this);
	}
	map.clear();
}
bool assert_peer_diritem(TLItem* item)
{
	TLItem** peer=item->GetPeerItem();
	return peer==NULL||*peer==NULL||dynamic_cast<TLItemDir*>(*peer)!=NULL;
}
void TLItemDir::Detach()
{
	assert(this!=NULL);
	if(this==NULL)
		return;
	subpairs->clear();
	assert(assert_peer_diritem(this));
	TLItem **_this,**_other;
	_other=GetPeerItem(&_this);
	if(_other==NULL||*_other==NULL)
		delete subpairs;
	else if(_this!=NULL)
		*_this=NULL;
	subpairs=NULL;
}
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
int TLItem::ToLineNum()
{
	TLItem *item=this;
	if(item==NULL)
		return -1;
	int iline=0;
	for(TLItemDir* dir=item->parent;dir!=NULL;item=dir,dir=dir->parent)
	{
		int i;
		for(i=0;i<(int)dir->subitems.size();i++)
		{
			if(item==dir->subitems[i])
				break;
			iline+=dir->subitems[i]->GetDispLength();
		}
		if(i==(int)dir->subitems.size())
			return -1;
		iline++;
	}
	return iline-1;
}
ListCtrlIterator::ListCtrlIterator(TLItem* root,int iline,TreeListCtrl* pList):m_pList(pList),m_pStkItem(NULL),lvl(0),m_iline(-1),end(false)
{
	if(root==NULL||iline<0)
		return;
	m_iline=iline;
	m_pStkItem=root->FromLineNum(m_iline,lvl);
	if(m_pStkItem==NULL)
		m_iline=-1;
}
ListCtrlIterator::ListCtrlIterator(ListCtrlIterator& other)
{
	memcpy(this,&other,sizeof(*this));
	other.m_pStkItem=NULL;
	other.lvl=0;
}
ListCtrlIterator::~ListCtrlIterator()
{
	free_item_stack(m_pStkItem);
}
ListCtrlIterator::operator bool()
{
	if(end)
		return false;
	end=(m_pStkItem==NULL||m_pList==NULL||m_pList->EndOfDraw(m_iline));
	return !end;
}
void ListCtrlIterator::operator++(int)
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
void ListCtrlIterator::operator--(int)
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
bool ListCtrlIterator::operator==(const ListCtrlIterator& other) const
{
	if(m_iline<0||other.m_iline<0)
		return false;
	return m_iline==other.m_iline;
}
bool ListCtrlIterator::operator!=(const ListCtrlIterator& other) const
{
	if(m_iline<0||other.m_iline<0)
		return false;
	return m_iline!=other.m_iline;
}
bool ListCtrlIterator::operator>(const ListCtrlIterator& other) const
{
	if(m_iline<0||other.m_iline<0)
		return false;
	return m_iline>other.m_iline;
}
bool ListCtrlIterator::operator<(const ListCtrlIterator& other) const
{
	if(m_iline<0||other.m_iline<0)
		return false;
	return m_iline<other.m_iline;
}
bool ListCtrlIterator::operator>=(const ListCtrlIterator& other) const
{
	if(m_iline<0||other.m_iline<0)
		return false;
	return m_iline>=other.m_iline;
}
bool ListCtrlIterator::operator<=(const ListCtrlIterator& other) const
{
	if(m_iline<0||other.m_iline<0)
		return false;
	return m_iline<=other.m_iline;
}
bool ItemSelector::IsFocus(int iline)
{
	if(m_iItemSel<0)
		return false;
	return iline==m_iItemSel;
}
bool ItemSelector::InDragRegion(int iline,bool* cancel)
{
	if(!valid(iline))
		return false;
	if(m_iDragStart<0||m_iDragEnd<0)
		return false;
	int start=m_iDragStart,end=m_iDragEnd;
	if(start>end)
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
	return item->issel;
}
void ItemSelector::SetSel(TLItem* item,int iline)
{
	for(set<SelItem>::iterator it=m_setSel.begin();it!=m_setSel.end();it++)
	{
		(*it).item->issel=false;
	}
	m_setSel.clear();
	m_iItemSel=-1;
	m_iDragStart=m_iDragEnd=-1;
	m_bCancelRgn=false;
	if(item==NULL)
		return;
	assert(valid(iline));
	AddSel(item,iline);
	BeginDragSel(iline,false);
	m_iItemSel=iline;
}
void ItemSelector::AddSel(TLItem* item,int iline)
{
	if(item==NULL)
		return;
	assert(valid(iline));
	assert(item->issel==false);
	assert(m_setSel.find(item)==m_setSel.end());
	item->issel=true;
	m_setSel.insert(SelItem(item,iline));
}
void ItemSelector::CancelSel(TLItem* item,int iline)
{
	if(item==NULL)
		return;
	assert(valid(iline));
	assert(item->issel==true);
	assert(m_setSel.find(item)!=m_setSel.end());
	item->issel=false;
	m_setSel.erase(SelItem(item,iline));
}
void ItemSelector::ToggleSel(TLItem* item,int iline)
{
	if(item==NULL)
		return;
	assert(valid(iline));
	EndDragSel();
	if(item->issel)
	{
		CancelSel(item,iline);
		BeginDragSel(iline,true);
	}
	else
	{
		AddSel(item,iline);
		BeginDragSel(iline,false);
	}
	m_iItemSel=iline;
}
bool ItemSelector::BeginDragSel(int iline,bool cancel)
{
	if(!valid(iline))
		return false;
	m_iDragStart=iline;
	m_bCancelRgn=cancel;
	return true;
}
bool ItemSelector::DragSelTo(int iline)
{
	if(!valid(iline))
		return false;
	if(m_iDragStart<0)
		return false;
	m_iDragEnd=iline;
	m_iItemSel=iline;
	return true;
}
void ItemSelector::EndDragSel()
{
	if(m_iDragStart<0||m_iDragEnd<0)
		goto end;
	{
		bool bRev=m_iDragStart>m_iDragEnd;
		ListCtrlIterator itstart(m_pOwner->m_pRootItem,m_iDragStart),
			itend(m_pOwner->m_pRootItem,m_iDragEnd);
		for(;bRev?itstart>=itend:itstart<=itend;bRev?itstart--:itstart++)
		{
			ItStkItem* stk=itstart.m_pStkItem;
			if(stk==NULL)
				continue;
			if(m_bCancelRgn)
			{
				if(stk->m_pLItem->issel)
					CancelSel(stk->m_pLItem,itstart.m_iline);
			}
			else
			{
				if(!stk->m_pLItem->issel)
					AddSel(stk->m_pLItem,itstart.m_iline);
			}
		}
	}
end:
	m_bCancelRgn=false;
	m_iDragStart=m_iDragEnd=-1;
}
void ItemSelector::SortSelection(SortedSelItemNode& tree)
{
	EndDragSel();
	map<int,SortedSelItemNode*> cache;
	tree.clear();
	for(set<SelItem>::iterator it=m_setSel.begin();it!=m_setSel.end();it++)
	{
		TLItem* item=it->item;
		int iline=it->iline;
		SortedSelItemNode* pcnode=NULL;
		while(item!=NULL)
		{
			map<int,SortedSelItemNode*>::iterator itmap=cache.find(iline);
			if(itmap!=cache.end())
			{
				assert(itmap->first==iline&&itmap->second->pItem==item);
				if(pcnode==NULL)
					continue;
				assert(itmap->second->map_sub.find(pcnode->iline)==
					itmap->second->map_sub.end());
				itmap->second->map_sub[pcnode->iline]=pcnode;
				break;
			}
			else
			{
				SortedSelItemNode* node;
				if(item->parent==NULL)
					node=&tree;
				else
					node=new SortedSelItemNode;
				node->pItem=item;
				node->iline=iline;
				if(pcnode!=NULL)
					node->map_sub[pcnode->iline]=pcnode;
				cache[iline]=node;
				pcnode=node;
			}
			assert(item->parent==NULL||item->parent->isopen);
			item=item->parent;
			iline=item->ToLineNum();
		}
	}
}
bool ItemSelector::valid(int iline)
{
	return iline>=0&&iline<(int)m_pOwner->m_nTotalLine;
}
void SortedSelItemNode::clear()
{
	for(map<int,SortedSelItemNode*>::iterator iter=map_sub.begin();iter!=map_sub.end();iter++)
	{
		delete iter->second;
	}
	map_sub.clear();
}
void SortedSelItemNode::extract()
{
	if(pItem->issel)
	{
		clear();
		return;
	}
	for(map<int,SortedSelItemNode*>::iterator iter=map_sub.begin();iter!=map_sub.end();iter++)
	{
		iter->second->extract();
	}
}