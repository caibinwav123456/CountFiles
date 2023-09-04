#include "pch.h"

CMap<UINT,UINT,CWnd*,CWnd*>* GetIDWndMap()
{
	static CMap<UINT,UINT,CWnd*,CWnd*> map;
	static bool init=false;
	if(!init)
	{
		map.InitHashTable(257);
		init=true;
	}
	return &map;
}
BOOL PDXAddToIDWndPtrMap(UINT id, CWnd* pWnd)
{
	CMap<UINT,UINT,CWnd*,CWnd*>* pMap=GetIDWndMap();
	CWnd* pOrigWnd;
	if(pMap->Lookup(id,pOrigWnd))
		return FALSE;
	pMap->SetAt(id,pWnd);
	return TRUE;
}
BOOL PDXRemoveFromIDWndPtrMap(UINT id)
{
	return GetIDWndMap()->RemoveKey(id);
}
CWnd* PDXGetWndFromID(UINT id)
{
	CWnd* pWnd;
	if(GetIDWndMap()->Lookup(id,pWnd))
		return pWnd;
	ASSERT(FALSE);
	return NULL;
}
