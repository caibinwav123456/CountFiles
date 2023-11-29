#include "pch.h"
#include "math.h"
#include "datetime.h"

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
#define clamp_value(val,minimum,maximum) \
	if(val<minimum) \
		val=minimum; \
	else if(val>maximum) \
		val=maximum;

CProgramData CProgramData::s_Data;
CProgramData::CProgramData()
	: m_dpiX(0)
	, m_dpiY(0)
	, m_deflogicX(96)
	, m_deflogicY(96)
	, m_scaleX(0)
	, m_scaleY(0)
	, m_strCachePath("LocalCache\\")
	, m_strHomePath("CaiBinSoft\\")
	, m_strExpPath("FileCleanerExport\\")
	, m_strCacheFileName("current")
	, m_strCFileExt(".fl")
	, m_strCFileErrExt(".err")
{
}
int CProgramData::Init()
{
	return s_Data.InitData();
}
int CProgramData::InitData()
{
	CDC dcScreen;
	dcScreen.CreateDC(_T("DISPLAY"),NULL,NULL,NULL);
	m_dpiX=dcScreen.GetDeviceCaps(LOGPIXELSX);
	m_dpiY=dcScreen.GetDeviceCaps(LOGPIXELSY);
	dcScreen.DeleteDC();

	const float min_scale=1.0f,max_scale=1.2f;
	m_scaleX=(float)m_dpiX/m_deflogicX;
	m_scaleY=(float)m_dpiY/m_deflogicY;
	clamp_value(m_scaleX,min_scale,max_scale)
	clamp_value(m_scaleY,min_scale,max_scale)

	m_strBasePath="D:\\";

	return 0;
}
int CProgramData::GetRealPixelsX(int logicx)
{
	return (int)roundf((float)logicx*s_Data.m_scaleX);
}
int CProgramData::GetRealPixelsY(int logicy)
{
	return (int)roundf((float)logicy*s_Data.m_scaleY);
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
	return GetProgramDataBasePath()+s_Data.m_strCachePath;
}
string CProgramData::GetProgramHomePath()
{
	return GetProgramDataBasePath()+s_Data.m_strHomePath;
}
string CProgramData::GetExportDirPath()
{
	return GetProgramHomePath()+s_Data.m_strExpPath;
}
string CProgramData::GetExportFilePath()
{
	CDateTime date;
	string strdate;
	sys_get_date_time(&date);
	date.Format(strdate,FORMAT_DATE|FORMAT_TIME,"","","");
	return GetExportDirPath()+strdate+s_Data.m_strCFileExt;
}
string CProgramData::GetCFilePathRoot()
{
	return GetCacheDirPath()+s_Data.m_strCacheFileName;
}
string CProgramData::GetCacheFilePath()
{
	return GetCFilePathRoot()+s_Data.m_strCFileExt;
}
string CProgramData::GetCacheErrFilePath()
{
	return GetCacheFilePath()+s_Data.m_strCFileErrExt;
}
string CProgramData::GetErrListFilePath(const string& path)
{
	return path+s_Data.m_strCFileErrExt;
}

#ifdef UNICODE
string ConvertTStrToAnsiStr(LPCTSTR from)
{
	char inbuf[64],*buf,*allocbuf=NULL;
	int length=WideCharToMultiByte(CP_ACP,0,from,-1,NULL,0,NULL,NULL);
	if(length<=0)
		return "";
	if(length<=64)
		buf=inbuf;
	else
		buf=allocbuf=new char[length];
	WideCharToMultiByte(CP_ACP,0,from,-1,buf,length,NULL,NULL);
	string to(buf);
	if(allocbuf!=NULL)
		delete[] allocbuf;
	return to;
}
CString ConvertAnsiStrToTStr(LPCSTR from)
{
	WCHAR inbuf[64],*buf,*allocbuf=NULL;
	int length=MultiByteToWideChar(CP_ACP,0,from,-1,NULL,0);
	if(length<=0)
		return L"";
	if(length<=64)
		buf=inbuf;
	else
		buf=allocbuf=new WCHAR[length];
	MultiByteToWideChar(CP_ACP,0,from,-1,buf,length);
	CString to(buf);
	if(allocbuf!=NULL)
		delete[] allocbuf;
	return to;
}
CString ConvertAnsiStrToTStr(const string& from)
{
	return ConvertAnsiStrToTStr(from.c_str());
}
#endif
