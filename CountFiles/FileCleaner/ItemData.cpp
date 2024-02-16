#include "pch.h"
#include "TreeListCtrl.h"
#include <assert.h>
inline TLItemSplice* TLItem::GetSplice()
{
	if(this==NULL)
		return NULL;
	return parent==NULL?NULL:parent->subpairs;
}
inline TLItemPair* TLItem::GetCouple()
{
	TLItemSplice* splice=GetSplice();
	if(splice==NULL)
		return NULL;
	if(parentidx<0)
		return NULL;
	return splice->jntitems[parentidx];
}
inline TLItemSplice* TLItemPair::GetSuper()
{
	assert_valid_tuple(this);
	TLItem* item=left!=NULL?left:right;
	return item->GetSplice();
}
TLItem** TLItem::GetPeerItem(TLItem*** _this)
{
	TLItemPair* tuple=GetCouple();
	if(tuple==NULL)
	{
		if(_this!=NULL)
			*_this=NULL;
		return NULL;
	}
	assert_valid_tuple(tuple);
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
bool assert_peer_diritem(TLItem* item)
{
	TLItem** peer=item->GetPeerItem();
	return peer==NULL||*peer==NULL||dynamic_cast<TLItemDir*>(*peer)!=NULL;
}
void TLItem::Detach()
{
	assert(this!=NULL);
	if(this==NULL)
		return;
	TLItem **_this;
	GetPeerItem(&_this);
	if(_this!=NULL)
		*_this=NULL;
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
	if(subpairs!=NULL&&(_other==NULL||*_other==NULL))
		delete subpairs;
	if(_this!=NULL)
		*_this=NULL;
	subpairs=NULL;
}
uint* TLItemDir::ptr_disp_len()
{
	return subpairs==NULL?&open_length:&subpairs->open_length;
}
uint TLItemDir::GetDispLength()
{
	uint* ptrlen=ptr_disp_len();
	return isopen?*ptrlen:1;
}
uint TLItemErrDir::GetDispLength()
{
	TLItem** peer=GetPeerItem();
	if(peer!=NULL&&*peer!=NULL&&(*peer)->type==eITypeDir)
		return ((TLItemDir*)*peer)->GetDispLength();
	return 1;
}
bool TLItemDir::IsBase()
{
	return parent==ctx->m_pTlUnit->m_pItemJoint;
}
void TLItemDir::update_displen(int diff)
{
	for(TLItemDir* pp=parent;
		pp!=ctx->m_pTlUnit->m_pItemJoint;
		pp=pp->parent)
	{
		uint* plen=pp->ptr_disp_len();
		assert(*plen>0);
		*plen+=diff;
	}
}
int TLItemDir::OpenDir(bool open,bool release)
{
	int ret=0;
	uint oldlen=GetDispLength();
	TLItem** peer=GetPeerItem();
	TLItemDir* peerdir=NULL;
	bool has_peer=false;
	if(peer!=NULL&&*peer!=NULL&&(*peer)->type==eITypeDir)
	{
		peerdir=(TLItemDir*)*peer;
		assert(peerdir->isopen==isopen);
		has_peer=true;
		peerdir->isopen=open;
	}
	isopen=open;
	uint* plen=ptr_disp_len();
	if(open)
	{
		if(*plen>0)
			goto end;
		fail_goto(ret,0,ctx->m_ListLoader.ExpandNode(dirnode,true,release),fail);
		if(has_peer)
			fail_goto(ret,0,peerdir->ctx->m_ListLoader.ExpandNode(peerdir->dirnode,true,release),fail);
		fail_goto(ret,0,construct_list(),fail);
		goto end;
	}
	else if(release)
	{
		clear();
		ret=ctx->m_ListLoader.ExpandNode(dirnode,false,true);
		int retc=0;
		if(has_peer)
			retc=peerdir->ctx->m_ListLoader.ExpandNode(peerdir->dirnode,false,true);
		if(ret==0)
			ret=retc;
		goto end;
	}
	else
	{
		fail_goto(ret,0,ctx->m_ListLoader.ExpandNode(dirnode,false),fail);
		if(has_peer)
			fail_goto(ret,0,peerdir->ctx->m_ListLoader.ExpandNode(dirnode,false),fail);
	}
	goto end;
fail:
	isopen=false;
	clear();
	ctx->m_ListLoader.ExpandNode(dirnode,false,true);
	if(has_peer)
		peerdir->ctx->m_ListLoader.ExpandNode(peerdir->dirnode,false,true);
end:
	if(parent==ctx->m_pTlUnit->m_pItemJoint||!parent->nested)
		update_displen(GetDispLength()-oldlen);
	return ret;
}
void TLItemDir::Release()
{
	OpenDir(false,true);
}
TLItem* ItStkItem::get_item(int _side)
{
	if(m_pJItem!=NULL)
	{
		if(_side<=0)
			return m_pJItem->left;
		else
			return m_pJItem->right;
	}
	else if(_side==side)
		return m_pItem;
	return NULL;
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
static inline void set_item_stack(ItStkItem* stk,TLItemDir* dir,uint idx,int side)
{
	if(dir->subpairs==NULL)
	{
		stk->m_pJItem=NULL;
		stk->m_pItem=dir->subitems[idx];
		stk->side=side;
		return;
	}
	stk->m_pJItem=dir->subpairs->jntitems[idx];
	if(stk->m_pJItem->left==NULL)
	{
		stk->m_pItem=stk->m_pJItem->right;
		stk->side=1;
	}
	else if(stk->m_pJItem->right==NULL)
	{
		stk->m_pItem=stk->m_pJItem->left;
		stk->side=-1;
	}
	else if(stk->m_pJItem->left->type==eITypeDir
		&&stk->m_pJItem->right->type!=eITypeDir)
	{
		stk->m_pItem=stk->m_pJItem->left;
		stk->side=-1;
	}
	else if(stk->m_pJItem->left->type!=eITypeDir
		&&stk->m_pJItem->right->type==eITypeDir)
	{
		stk->m_pItem=stk->m_pJItem->right;
		stk->side=1;
	}
	else
	{
		stk->m_pItem=(side<=0?stk->m_pJItem->left:stk->m_pJItem->right);
		stk->side=side;
	}
}
size_t TLItemDir::size() const
{
	if(subpairs!=NULL)
		return subpairs->jntitems.size();
	return subitems.size();
}
ItStkItem* TLItemDir::FromLineNum(int iline,int& lvl,int side)
{
	if(iline<0)
		return NULL;
	TLItemDir* cur=this;
	ItStkItem* stack=push_item_stack(NULL);
	int iacc=0;
	int level=0;
	for(;;)
	{
		if(stack->parentidx>=(int)cur->size())
		{
			free_item_stack(stack);
			return NULL;
		}
		set_item_stack(stack,cur,stack->parentidx,side);
		assert(stack->m_pItem!=NULL);
		side=stack->side;
		if(iacc==iline)
		{
			lvl=level;
			return stack;
		}
		int displen=stack->m_pItem->GetDispLength();
		if(iacc+displen>iline)
		{
			iacc++;
			cur=dynamic_cast<TLItemDir*>(stack->m_pItem);
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
	assert(this!=NULL);
	if(this==NULL)
		return -1;
	int iline=0;
	for(TLItem *item=this;!item->IsBase();item=item->parent)
	{
		TLItemDir* dir=item->parent;
		if(item->parentidx<0)
			return -1;
		for(int i=0;i<item->parentidx;i++)
		{
			if(dir->subpairs!=NULL)
			{
				TLItemPair* tuple=dir->subpairs->jntitems[i];
				iline+=(tuple->left!=NULL?tuple->left:tuple->right)->GetDispLength();
			}
			else
				iline+=dir->subitems[i]->GetDispLength();
		}
		iline++;
	}
	return iline-1;
}
ListCtrlIterator::ListCtrlIterator(TLItemDir* root,int iline,int side,TreeListCtrl* pList):m_pList(pList),m_pStkItem(NULL),lvl(0),m_iline(-1),end(false)
{
	if(root==NULL||iline<0)
		return;
	m_iline=iline;
	m_pStkItem=root->FromLineNum(m_iline,lvl,side);
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
	int side=m_pStkItem->side;
	if(m_pStkItem->m_pItem->type==eITypeDir)
	{
		TLItemDir* dir=dynamic_cast<TLItemDir*>(m_pStkItem->m_pItem);
		assert(dir!=NULL);
		if(dir->isopen&&dir->size()>0)
		{
			m_pStkItem=push_item_stack(m_pStkItem);
			set_item_stack(m_pStkItem,dir,0,side);
			lvl++;
			return;
		}
	}
	for(;;)
	{
		if(m_pStkItem->parentidx+1<(int)m_pStkItem->m_pItem->parent->size())
		{
			m_pStkItem->parentidx++;
			set_item_stack(m_pStkItem,m_pStkItem->m_pItem->parent,m_pStkItem->parentidx,side);
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
	int side=m_pStkItem->side;
	if(end)
	{
		assert(lvl==0&&m_pStkItem->next==NULL);
		end=false;
		goto second_phase;
	}
	if(m_pStkItem->parentidx>0)
	{
		m_pStkItem->parentidx--;
		set_item_stack(m_pStkItem,m_pStkItem->m_pItem->parent,m_pStkItem->parentidx,side);
		side=m_pStkItem->side;
		goto second_phase;
	}
	if(lvl==0)
		return;
	lvl--;
	ItStkItem* next=m_pStkItem->next;
	delete m_pStkItem;
	m_pStkItem=next;
	return;
second_phase:
	for(;;)
	{
		if(m_pStkItem->m_pItem->type!=eITypeDir)
			break;
		TLItemDir* dir=dynamic_cast<TLItemDir*>(m_pStkItem->m_pItem);
		assert(dir!=NULL);
		if((!dir->isopen)||dir->size()==0)
			break;
		m_pStkItem=push_item_stack(m_pStkItem);
		m_pStkItem->parentidx=dir->size()-1;
		set_item_stack(m_pStkItem,dir,dir->size()-1,side);
		side=m_pStkItem->side;
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
int ItemSelector::GetFocus()
{
	return m_iItemSel;
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
void ItemSelector::SetSel(ItStkItem* item,int iline)
{
	for(set<SelItem>::iterator it=m_setSel.begin();it!=m_setSel.end();it++)
	{
		SelItem& selitem=(SelItem&)*it;
		if(selitem.pair!=NULL)
		{
			if(selitem.pair->left!=NULL)
				selitem.pair->left->issel=false;
			if(selitem.pair->right!=NULL)
				selitem.pair->right->issel=false;
		}
		else
			selitem.item->issel=false;
	}
	m_setSel.clear();
	m_iItemSel=-1;
	m_iDragStart=m_iDragEnd=-1;
	m_bCancelRgn=false;
	if(item==NULL||!valid(iline))
		return;
	AddSel(item,iline);
	BeginDragSel(iline,false);
	m_iItemSel=iline;
}
void ItemSelector::AddSel(ItStkItem* item,int iline)
{
	if(item==NULL)
		return;
	assert(valid(iline));
	assert(m_setSel.find(SelItem(item,iline))==m_setSel.end());
	if(item->m_pJItem!=NULL)
	{
		if(item->m_pJItem->left!=NULL)
		{
			assert(!item->m_pJItem->left->issel);
			item->m_pJItem->left->issel=true;
		}
		if(item->m_pJItem->right!=NULL)
		{
			assert(!item->m_pJItem->right->issel);
			item->m_pJItem->right->issel=true;
		}
	}
	else
	{
		assert(!item->m_pItem->issel);
		item->m_pItem->issel=true;
	}
	m_setSel.insert(SelItem(item,iline));
}
void ItemSelector::CancelSel(ItStkItem* item,int iline)
{
	if(item==NULL)
		return;
	assert(valid(iline));
	assert(m_setSel.find(SelItem(item,iline))!=m_setSel.end());
	if(item->m_pJItem!=NULL)
	{
		if(item->m_pJItem->left!=NULL)
		{
			assert(item->m_pJItem->left->issel);
			item->m_pJItem->left->issel=false;
		}
		if(item->m_pJItem->right!=NULL)
		{
			assert(item->m_pJItem->right->issel);
			item->m_pJItem->right->issel=false;
		}
	}
	else
	{
		assert(item->m_pItem->issel);
		item->m_pItem->issel=false;
	}
	m_setSel.erase(SelItem(item,iline));
}
void ItemSelector::ToggleSel(ItStkItem* item,int iline)
{
	if(item==NULL)
		return;
	assert(valid(iline));
	EndDragSel();
	if(item->m_pItem->issel)
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
	m_iDragEnd=iline;
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
bool ItemSelector::CompoundSel(int iline)
{
	if(!valid(iline))
		return false;
	if(m_iDragStart<0)
		return false;
	DragSelTo(iline);
	EndDragSel();
	BeginDragSel(iline,false);
	m_iItemSel=iline;
	return true;
}
bool ItemSelector::ClearAndDragSel(ItStkItem* item,int iline)
{
	if(item==NULL||!valid(iline))
		return false;
	if(m_iDragStart<0||m_iItemSel<0)
	{
		SetSel(item,iline);
		return true;
	}
	else if(m_iDragStart>=0)
	{
		int pos=m_iDragStart;
		ListCtrlIterator it=m_pOwner->GetListIter(pos);
		if(it.m_pStkItem==NULL)
			return false;
		SetSel(it.m_pStkItem,pos);
		DragSelTo(iline);
		return true;
	}
	return false;
}
void ItemSelector::EndDragSel()
{
	if(m_iDragStart<0||m_iDragEnd<0)
		goto end;
	{
		bool bRev=m_iDragStart>m_iDragEnd;
		ListCtrlIterator itstart=m_pOwner->GetListIter(m_iDragStart),
			itend=m_pOwner->GetListIter(m_iDragEnd);
		for(;bRev?itstart>=itend:itstart<=itend;bRev?itstart--:itstart++)
		{
			ItStkItem* stk=itstart.m_pStkItem;
			if(stk==NULL)
				continue;
			if(m_bCancelRgn)
			{
				if(stk->m_pItem->issel)
					CancelSel(stk,itstart.m_iline);
			}
			else
			{
				if(!stk->m_pItem->issel)
					AddSel(stk,itstart.m_iline);
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
		ItemSelector::SelItem item=*it;
		int iline=item.iline;
		int side=item.side;
		SortedSelItemNode* pcnode=NULL;
		bool isbase=false;
		do
		{
			map<int,SortedSelItemNode*>::iterator itmap=cache.find(iline);
			if(itmap!=cache.end())
			{
				assert(itmap->first==iline&&itmap->second->item.pair==item.pair);
				if(pcnode==NULL)
					continue;
				assert(itmap->second->map_sub.find(pcnode->item.iline)==
					itmap->second->map_sub.end());
				itmap->second->map_sub[pcnode->item.iline]=pcnode;
				break;
			}
			else
			{
				SortedSelItemNode* node;
				if(item.item->parent==NULL)
					node=&tree;
				else
					node=new SortedSelItemNode;
				node->item=item;
				if(pcnode!=NULL)
					node->map_sub[pcnode->item.iline]=pcnode;
				cache[iline]=node;
				pcnode=node;
			}
			assert(item.item->parent==NULL||item.item->parent->isopen);
			isbase=item.item->IsBase();
			if(isbase)
				continue;
			item.item=item.item->parent;
			item.pair=item.item->GetCouple();
			item.side=side;
			iline=item.iline=item.item->ToLineNum();
		}while(!isbase);
	}
}
bool ItemSelector::valid(int iline)
{
	return iline>=0&&iline<(int)m_pOwner->m_pCurTlU->m_nTotalLine;
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
	if(item.item->issel)
	{
		clear();
		return;
	}
	for(map<int,SortedSelItemNode*>::iterator iter=map_sub.begin();iter!=map_sub.end();iter++)
	{
		iter->second->extract();
	}
}