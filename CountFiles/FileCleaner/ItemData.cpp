#include "pch.h"
#include "TreeListCtrl.h"
#include "resource.h"
uint TLItemDir::GetDispLength()
{
	return isopen?open_length:1;
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
