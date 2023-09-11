#ifndef _PROGRAM_DATA_H_
#define _PROGRAM_DATA_H_
#include <string>
using namespace std;

#define t2a(p) ConvertTStrToAnsiStr(p).c_str()
#define a2t(p) (LPCTSTR)ConvertAnsiStrToTStr(p)
#define t2astr(p) ConvertTStrToAnsiStr(p)
#define a2tstr(p) ConvertAnsiStrToTStr(p)

string ConvertTStrToAnsiStr(LPCTSTR from);
CString ConvertAnsiStrToTStr(LPCSTR from);
CString ConvertAnsiStrToTStr(const string& from);

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
	static ID2CWndPtrAssoc* GetWndAssoc(); \
	CAddToWndMap<className> __thisMapObj;

#define IMPLEMENT_ID2WND_MAP(className,id) \
	ID2CWndPtrAssoc* className::GetWndAssoc() \
	{ \
		static ID2CWndPtrAssoc theAssoc(id,reinterpret_cast<INT_PTR> \
			(&(static_cast<className*>((CWnd*)NULL))->__thisMapObj)); \
		return &theAssoc; \
	}

class CProgramData
{
public:
	static string GetProgramDataBasePath();
	static string GetCacheDirPath();
	static string GetCFilePathRoot();
	static string GetCacheFilePath();
	static string GetCacheErrFilePath();
	static string GetErrListFilePath(const string& path);
private:
	CProgramData::CProgramData();
	const string m_strBasePath;
	const string m_strCachePath;
	const string m_strCacheFileName;
	const string m_strCFileExt;
	const string m_strCFileErrExt;
	static CProgramData s_Data;
};

#endif
