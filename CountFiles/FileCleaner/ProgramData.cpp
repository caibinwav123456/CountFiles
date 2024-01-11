#include "pch.h"
#include <math.h>
#include "defines.h"
#include "datetime.h"
#include "algor_templ.h"
#include "utility.h"
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#define safe_release(ptr) \
	if(ptr!=NULL) \
	{ \
		delete ptr; \
		ptr=NULL; \
	}
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
class CBaseTree;
class CBaseList;
struct PathNodeList:public PathNode
{
	PathNodeList* prev;
	PathNodeList* next;
	string extern_path;
	PathNodeList():prev(this),next(this){}
	virtual void Release();
	virtual string GetPathInternal();
	virtual PathNode* DupInternal();
	virtual PathNode* GetSub(const string& name);
	virtual PathNode* GetSibling(const string& name);
	virtual bool PeekSub(const string& name);
	void Remove();
};
struct PathNodeTree:public PathNode
{
	KeyTree<string,PathNodeTree>::TreeNode* node;
	CBaseTree* hosttree;
	PathNodeTree(KeyTree<string,PathNodeTree>::TreeNode* _node=NULL,
		CBaseTree* _hosttree=NULL):node(_node),hosttree(_hosttree){}
	virtual void Release();
	virtual string GetPathInternal();
	virtual PathNode* DupInternal();
	virtual PathNode* GetSub(const string& name);
	virtual PathNode* GetSibling(const string& name);
	virtual bool PeekSub(const string& name);
};
class CBaseList
{
	PathNodeList first;
	PathNodeList last;
public:
	CBaseList()
	{
		first.prev=NULL;
		first.next=&last;
		last.prev=&first;
		last.next=NULL;
	}
	~CBaseList()
	{
		Clear();
	}
	void Clear();
	void AddNode(PathNodeList* node);
};
inline string process_path(const string& input)
{
	if(input.empty())
		return "";
	return input.back()=='\\'?input.substr(0,input.size()-1):input;
}
class CBaseTree:public KeyTree<string,PathNodeTree>
{
	friend struct PathNodeTree;
	friend struct PathNodeList;
	string base_path;
public:
	CBaseTree(const string& base,const string& name):KeyTree<string,PathNodeTree>(process_path(name)),base_path(process_path(base))
	{
		KeyTree<string,PathNodeTree>::TreeNode* node=GetRootNode();
		node->t.node=node;
		node->t.hosttree=this;
	}
};
void PathNodeList::Release()
{
	if((--ref)==0)
	{
		Remove();
		delete this;
	}
}
string PathNodeList::GetPathInternal()
{
	return extern_path;
}
PathNode* PathNodeList::DupInternal()
{
	ref++;
	return this;
}
PathNode* PathNodeList::GetSub(const string& name)
{
	PathNodeList* node=new PathNodeList;
	node->extern_path=extern_path+"\\"+name;
	CProgramData::GetPathList()->AddNode(node);
	return node;
}
PathNode* PathNodeList::GetSibling(const string& name)
{
	int pos=extern_path.rfind('\\');
	string base=pos==string::npos?extern_path:extern_path.substr(0,pos+1);
	PathNodeList* node=new PathNodeList;
	node->extern_path=base+name;
	CProgramData::GetPathList()->AddNode(node);
	return node;
}
bool PathNodeList::PeekSub(const string& name)
{
	string path=extern_path+"\\"+name;
	return sys_fstat((char*)path.c_str(),NULL)==0;
}
void PathNodeList::Remove()
{
	prev->next=next;
	next->prev=prev;
	prev=next=this;
}
void PathNodeTree::Release()
{
	for(KeyTree<string,PathNodeTree>::TreeNode* pnode=node;pnode!=NULL;)
	{
		KeyTree<string,PathNodeTree>::TreeNode* tmp=pnode;
		pnode=pnode->GetParent();
		if((--tmp->t.ref)==0)
		{
			sys_fdelete((char*)tmp->t.GetPath().c_str());
			tmp->Detach();
			delete tmp;
		}
	}
}
string PathNodeTree::GetPathInternal()
{
	vector<string*> vname;
	for(KeyTree<string,PathNodeTree>::TreeNode* pnode=node;pnode!=NULL;pnode=pnode->GetParent())
		vname.push_back(&pnode->key);
	reverse(vname.begin(),vname.end());
	string path=node->t.hosttree->base_path;
	for(vector<string*>::iterator it=vname.begin();it<vname.end();it++)
		path+=(string("\\")+**it);
	return path;
}
PathNode* PathNodeTree::DupInternal()
{
	for(KeyTree<string,PathNodeTree>::TreeNode* pnode=node;pnode!=NULL;pnode=pnode->GetParent())
		pnode->t.ref++;
	return this;
}
PathNode* PathNodeTree::GetSub(const string& name)
{
	KeyTree<string,PathNodeTree>::TreeNode* pnode;
	PathNodeTree* pathnode;
	if((pnode=node->GetChild(name))!=NULL)
	{
		pathnode=&pnode->t;
		pathnode->Dup();
	}
	else
	{
		pnode=new KeyTree<string,PathNodeTree>::TreeNode(name);
		pathnode=&pnode->t;
		pathnode->node=pnode;
		pathnode->hosttree=hosttree;
		pnode->AddTo(node);
		Dup();
	}
	return pathnode;
}
PathNode* PathNodeTree::GetSibling(const string& name)
{
	KeyTree<string,PathNodeTree>::TreeNode* pnode=node->GetParent();
	return pnode->t.GetSub(name);
}
bool PathNodeTree::PeekSub(const string& name)
{
	return node->GetChild(name)!=NULL;
}
void CBaseList::AddNode(PathNodeList* node)
{
	PathNodeList* next=first.next;
	first.next=next->prev=node;
	node->prev=&first;
	node->next=next;
}
void CBaseList::Clear()
{
	while(first.next!=&last)
	{
		PathNodeList* pnode=first.next;
		pnode->Remove();
		delete pnode;
	}
}
#define clamp_value(val,minimum,maximum) \
	if(val<minimum) \
		val=minimum; \
	else if(val>maximum) \
		val=maximum;

CProgramData CProgramData::s_Data;
CProgramData::CProgramData()
	: m_pBaseTree(NULL)
	, m_pBaseList(NULL)
	, m_dpiX(0)
	, m_dpiY(0)
	, m_deflogicX(96)
	, m_deflogicY(96)
	, m_scaleX(0)
	, m_scaleY(0)
	, m_strCachePath("LocalCache\\")
	, m_strTempPath("Temp\\")
	, m_strBackupPath("Backup\\")
	, m_strHomePath("CaiBinSoft\\")
	, m_strExpPath("FileCleanerExport\\")
	, m_strCacheFileName("current")
	, m_strCFileExt(".fl")
	, m_strCFileErrExt(".err")
{
}
CProgramData::~CProgramData()
{
	ExitData();
}
int CProgramData::Init()
{
	return s_Data.InitData();
}
void CProgramData::Exit()
{
	s_Data.ExitData();
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

	m_pBaseTree=new CBaseTree(GetCacheDirPath(),m_strTempPath);
	m_pBaseList=new CBaseList;

	int ret=0;
	fail_op(ret,0,sys_mkdir((char*)GetTempDirPath().c_str()),
	{
		PDXShowMessage(_T("Failed to create temporary path: %s"),a2t(get_error_desc(ret)));
		return ret;
	});
	fail_op(ret,0,sys_mkdir((char*)GetBackupDirPath().c_str()),
	{
		PDXShowMessage(_T("Failed to create backup path: %s"),a2t(get_error_desc(ret)));
		return ret;
	});

	srand(time(NULL));

	return 0;
}
inline string get_app_exe_name()
{
	char buf[256];
	GetModuleFileNameA(NULL,buf,256);
	string exe_path=buf;
	int pos=exe_path.rfind('\\');
	return pos==string::npos?exe_path:exe_path.substr(pos+1);
}
void CProgramData::ExitData()
{
	safe_release(m_pBaseTree);
	safe_release(m_pBaseList);
	if(!sys_has_dup_process((char*)get_app_exe_name().c_str()))
		sys_recurse_fdelete((char*)GetTempDirPath().c_str(),NULL);
}
CBaseList* CProgramData::GetPathList()
{
	return s_Data.m_pBaseList;
}
CBaseTree* CProgramData::GetPathTree()
{
	return s_Data.m_pBaseTree;
}
PathNode* CProgramData::GetBasePathNode()
{
	KeyTree<string,PathNodeTree>::TreeNode* pnode=s_Data.m_pBaseTree->GetRootNode();
	return &pnode->t;
}
PathNode* CProgramData::GetPathNode(const string& path)
{
	PathNodeList* node=new PathNodeList;
	node->extern_path=path;
	s_Data.m_pBaseList->AddNode(node);
	return node;
}
PathNode* CProgramData::GetErrListFileNode(PathNode* node)
{
	string filepath=node->GetPath();
	int pos=filepath.rfind('\\');
	string filename=pos==string::npos?filepath:filepath.substr(pos+1);
	return node->GetSibling(GetErrListFilePath(filename));
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
string CProgramData::GetTempDirPath()
{
	return GetCacheDirPath()+s_Data.m_strTempPath;
}
string CProgramData::GetBackupDirPath()
{
	return GetCacheDirPath()+s_Data.m_strBackupPath;
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
string CProgramData::GetCFileRoot()
{
	return s_Data.m_strCacheFileName;
}
string CProgramData::GetCacheFileExt()
{
	return s_Data.m_strCFileExt;
}
string CProgramData::GetCacheErrFileExt()
{
	return s_Data.m_strCFileErrExt;
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
