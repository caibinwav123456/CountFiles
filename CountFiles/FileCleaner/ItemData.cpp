#include "pch.h"
#include "TreeListCtrl.h"
#include "resource.h"
ListCtrlDrawIterator::ListCtrlDrawIterator(ListCtrlDrawIterator& iter)
{
	memcpy(this,&iter,sizeof(*this));
	iter.m_pStkItem=NULL;
	iter.lvl=0;
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
