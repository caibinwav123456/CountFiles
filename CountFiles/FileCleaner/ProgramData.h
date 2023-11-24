#ifndef _PROGRAM_DATA_H_
#define _PROGRAM_DATA_H_
#include <string>
using namespace std;

#ifdef UNICODE
#define t2a(p) ConvertTStrToAnsiStr(p).c_str()
#define a2t(p) (LPCTSTR)ConvertAnsiStrToTStr(p)
#define t2astr(p) ConvertTStrToAnsiStr(p)
#define a2tstr(p) ConvertAnsiStrToTStr(p)

string ConvertTStrToAnsiStr(LPCTSTR from);
CString ConvertAnsiStrToTStr(LPCSTR from);
CString ConvertAnsiStrToTStr(const string& from);
#else
inline LPCSTR t2a(LPSTR p)
{
	return p;
}
inline LPCSTR a2t(LPSTR p)
{
	return p;
}
inline LPCSTR a2t(const string& str)
{
	return str.c_str();
}
inline string t2astr(const CString& str)
{
	return (LPCSTR)str;
}
inline CString a2tstr(const string& str)
{
	return str.c_str();
}
#endif

struct ID2CWndPtrAssoc
{
	UINT id;
	INT_PTR offset;
	ID2CWndPtrAssoc(UINT _id,INT_PTR _off):id(_id),offset(_off){}
};
BOOL PDXAddToIDWndPtrMap(UINT id,CWnd* pWnd);
BOOL PDXRemoveFromIDWndPtrMap(UINT id);
CWnd* PDXGetWndFromID(UINT id);
void PDXShowMessage(LPCTSTR format,...);
template<typename T>
class CAddToWndMap
{
public:
	CAddToWndMap()
	{
		ID2CWndPtrAssoc* pAssoc=T::GetWndAssoc();
		VERIFY(PDXAddToIDWndPtrMap(pAssoc->id,(CWnd*)(((INT_PTR)this)-pAssoc->offset)));
	}
	~CAddToWndMap()
	{
		ID2CWndPtrAssoc* pAssoc=T::GetWndAssoc();
		PDXRemoveFromIDWndPtrMap(pAssoc->id);
	}
};
#define DECLARE_ID2WND_MAP(className) \
	friend class CAddToWndMap<className>; \
	static ID2CWndPtrAssoc* GetWndAssoc(); \
	CAddToWndMap<className> __thisMapObj;

#define IMPLEMENT_ID2WND_MAP(className,id) \
	ID2CWndPtrAssoc* className::GetWndAssoc() \
	{ \
		static ID2CWndPtrAssoc theAssoc(id,reinterpret_cast<INT_PTR> \
			(&(static_cast<className*>((CWnd*)NULL))->__thisMapObj)); \
		return &theAssoc; \
	}
inline LRESULT SendMessageToIDWnd(UINT id,UINT message,WPARAM wParam=0,LPARAM lParam=0)
{
	CWnd* pWnd=PDXGetWndFromID(id);
	if(pWnd->GetSafeHwnd()!=NULL)
		return pWnd->SendMessage(message,wParam,lParam);
	return -1;
}
class CProgramData
{
public:
	static int Init();
	static string GetProgramDataBasePath();
	static string GetCacheDirPath();
	static string GetExportDirPath();
	static string GetExportFilePath();
	static string GetCFilePathRoot();
	static string GetCacheFilePath();
	static string GetCacheErrFilePath();
	static string GetErrListFilePath(const string& path);

	static int GetRealPixelsX(int logicx);
	static int GetRealPixelsY(int logicy);

	static CPoint GetRealPoint(POINT pt);
	static CRect GetRealRect(RECT rect);

private:
	CProgramData::CProgramData();
	int InitData();
	const int m_deflogicX;
	const int m_deflogicY;
	int m_dpiX;
	int m_dpiY;
	float m_scaleX;
	float m_scaleY;
	string m_strBasePath;
	const string m_strCachePath;
	const string m_strCacheFileName;
	const string m_strExpPath;
	const string m_strCFileExt;
	const string m_strCFileErrExt;
	static CProgramData s_Data;
};

#endif
