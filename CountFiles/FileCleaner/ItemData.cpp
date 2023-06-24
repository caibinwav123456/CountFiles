#include "pch.h"
#include "TreeListCtrl.h"
#include "resource.h"
ListCtrlDrawIterator::operator bool()
{
	return !m_pList->EndOfDraw(m_iline);
}
void ListCtrlDrawIterator::operator++(int)
{
	m_iline++;
}
