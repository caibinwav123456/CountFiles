#include "pch.h"
#include "TreeListCtrl.h"
#include "resource.h"
#include <assert.h>
uint TLItemDir::GetDispLength()
{
	return isopen?open_length:1;
}
int TLItemDir::OpenDir(bool open,bool release)
{
	int ret=0;
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
		return 0;
	}
	else if(release)
	{
		open_length=0;
		clear();
		return ctx->ExpandNode(dirnode,false,true);
	}
	else if(0!=(ret=ctx->ExpandNode(dirnode,false)))
		goto fail;
	return 0;
fail:
	isopen=false;
	open_length=0;
	clear();
	return ret;
}
int TLItemDir::construct_list()
{
	int ret=0;
	assert(open_length==0);
	assert(subitems.empty());
	assert(subdirs.empty());
	assert(subfiles.empty());
	assert(errdirs.empty());
	assert(errfiles.empty());

	return 0;
}
void TLItemDir::clear()
{
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
ListCtrlDrawIterator::ListCtrlDrawIterator(ListCtrlDrawIterator& other)
{
	memcpy(this,&other,sizeof(*this));
	other.m_pStkItem=NULL;
	other.lvl=0;
}
ListCtrlDrawIterator::~ListCtrlDrawIterator()
{
	while(m_pStkItem!=NULL)
	{
		ItStkItem* item=m_pStkItem;
		m_pStkItem=m_pStkItem->next;
		delete item;
	}
}
ListCtrlDrawIterator::operator bool()
{
	return !m_pList->EndOfDraw(m_iline);
}
void ListCtrlDrawIterator::operator++(int)
{
	m_iline++;
}
void ListCtrlDrawIterator::operator--(int)
{
	if(m_iline>=0)
		m_iline--;
}
bool ListCtrlDrawIterator::operator==(const ListCtrlDrawIterator& other) const
{
	return m_iline==other.m_iline;
}
