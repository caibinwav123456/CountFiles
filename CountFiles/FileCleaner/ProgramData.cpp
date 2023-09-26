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
void PDXShowMessage(LPCTSTR format,...)
{
	CString strMsg;
	va_list args;
	va_start(args, format);
	strMsg.FormatV(format, args);
	va_end(args);
	AfxMessageBox(strMsg);
}

CProgramData CProgramData::s_Data;
CProgramData::CProgramData()
	: m_dpiX(0)
	, m_dpiY(0)
	, m_deflogicX(96)
	, m_deflogicY(96)
	, m_strCachePath("LocalCache\\")
	, m_strCacheFileName("current")
	, m_strCFileExt(".fl")
	, m_strCFileErrExt(".err")
{
}
int CProgramData::Init()
{
	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"),NULL,NULL,NULL);
	s_Data.m_dpiX=dcScreen.GetDeviceCaps(LOGPIXELSX);
	s_Data.m_dpiY=dcScreen.GetDeviceCaps(LOGPIXELSY);
	dcScreen.DeleteDC();

	s_Data.m_strBasePath="D:\\";

	return 0;
}
int CProgramData::GetRealPixelsX(int logicx)
{
	return logicx*s_Data.m_dpiX/s_Data.m_deflogicX;
}
int CProgramData::GetRealPixelsY(int logicy)
{
	return logicy*s_Data.m_dpiY/s_Data.m_deflogicY;
}
CPoint CProgramData::GetRealPoint(POINT pt)
{
	return CPoint(GetRealPixelsX(pt.x),GetRealPixelsY(pt.y));
}
CRect CProgramData::GetRealRect(RECT rect)
{
	return CRect(GetRealPoint(CRect(rect).TopLeft()),GetRealPoint(CRect(rect).BottomRight()));
}
string CProgramData::GetProgramDataBasePath()
{
	return s_Data.m_strBasePath;
}
string CProgramData::GetCacheDirPath()
{
	return s_Data.GetProgramDataBasePath()+s_Data.m_strCachePath;
}
string CProgramData::GetCFilePathRoot()
{
	return s_Data.GetCacheDirPath()+s_Data.m_strCacheFileName;
}
string CProgramData::GetCacheFilePath()
{
	return s_Data.GetCFilePathRoot()+s_Data.m_strCFileExt;
}
string CProgramData::GetCacheErrFilePath()
{
	return s_Data.GetCacheFilePath()+s_Data.m_strCFileErrExt;
}
string CProgramData::GetErrListFilePath(const string& path)
{
	return path+s_Data.m_strCFileErrExt;
}

#ifdef UNICODE
string ConvertTStrToAnsiStr(LPCTSTR from)
{
	USES_CONVERSION;
	return T2A(from);
}
CString ConvertAnsiStrToTStr(LPCSTR from)
{
	USES_CONVERSION;
	return A2T(from);
}
CString ConvertAnsiStrToTStr(const string& from)
{
	USES_CONVERSION;
	return A2T(from.c_str());
}
#endif