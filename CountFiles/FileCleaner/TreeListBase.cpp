#include "pch.h"
#include "TreeListCtrl.h"
#include "utility.h"
#include <assert.h>
#define safe_release(ptr) \
	if(ptr!=NULL) \
	{ \
		ptr->Release(); \
		ptr=NULL; \
	}
bool ListFileNode::valid() const
{
	if(pListNode==NULL||pListNode->GetPath().empty())
		return pErrNode==NULL;
	return pErrNode==NULL||!pErrNode->GetPath().empty();
}
bool ListFileNode::empty() const
{
	assert(valid());
	return (!pListNode)||pListNode->GetPath().empty();
}
void ListFileNode::Release()
{
	safe_release(pListNode);
	safe_release(pErrNode);
}
void ListFileNode::Curl()
{
	assert(valid());
	if(pErrNode!=NULL&&sys_fstat((char*)pErrNode->GetPath().c_str(),NULL)!=0)
		safe_release(pErrNode);
}
void TLUnit::Curl()
{
	m_treeLeft.m_lfNode.Curl();
	m_treeRight.m_lfNode.Curl();
}
int TLUnit::PrepareBase()
{
	UInteger64 uuid;
	random_integer64(uuid);
	string idstr=FormatI64Hex(uuid);
	while(CProgramData::GetBasePathNode()->PeekSub(idstr)
		||sys_fstat((char*)(CProgramData::GetBasePathNode()->GetPath()+"\\"+idstr).c_str(),NULL)==0)
	{
		uuid+=UInteger64(1);
		idstr=FormatI64Hex(uuid);
	}
	m_pBaseDirNode=CProgramData::GetBasePathNode()->GetSub(idstr);
	return sys_mkdir((char*)m_pBaseDirNode->GetPath().c_str());
}
void TLUnit::DestroyBase()
{
	safe_release(m_pBaseDirNode);
}
void TLUnit::CacheNode()
{
	assert(m_CacheNode.pListNode==NULL
		&&m_CacheNode.pErrNode==NULL);
	m_CacheNode.pListNode=m_treeLeft.m_lfNode.pListNode->Dup();
	m_CacheNode.pErrNode=m_treeLeft.m_lfNode.pErrNode->Dup();
}
void TLUnit::RestoreNode()
{
	assert(m_treeLeft.m_lfNode.pListNode==NULL
		&&m_treeLeft.m_lfNode.pErrNode==NULL);
	m_treeLeft.m_lfNode=m_CacheNode;
	memset(&m_CacheNode,0,sizeof(ListFileNode));
}
void TLUnit::DestroyCacheNode()
{
	m_CacheNode.Release();
}
void TLUnit::Fork()
{
	char buf[50];
	for(int i=0;;i++)
	{
		sprintf(buf,"%s%d%s",CProgramData::GetCFileRoot().c_str(),i,
			CProgramData::GetCacheFileExt().c_str());
		if(!m_pBaseDirNode->PeekSub(buf))
			break;
	}
	assert(m_treeLeft.m_lfNode.pListNode==NULL
		&&m_treeLeft.m_lfNode.pErrNode==NULL);
	m_treeLeft.m_lfNode.pListNode=m_pBaseDirNode->GetSub(buf);
	m_treeLeft.m_lfNode.pErrNode=m_pBaseDirNode->GetSub(
		CProgramData::GetErrListFilePath(buf));
}
static inline void PrepFile(ListFileNode* node,const char* file)
{
	assert(node->pListNode==NULL&&node->pErrNode==NULL);
	if(file==NULL||*file==0)
		return;
	node->pListNode=CProgramData::GetPathNode(file);
	node->pErrNode=CProgramData::GetErrListFileNode(node->pListNode);
	node->Curl();
}
int TreeListCtrl::LoadBase(UINT mask,const char* lfile,const char* rfile)
{
	PrepFile(&m_TlU.m_treeLeft.m_lfNode,lfile);
	PrepFile(&m_TlU.m_treeRight.m_lfNode,rfile);
	string lerr=m_TlU.m_treeLeft.m_lfNode.pErrNode->GetPath();
	string rerr=m_TlU.m_treeRight.m_lfNode.pErrNode->GetPath();
	return Load(mask,lfile,lerr.c_str(),rfile,rerr.c_str());
}
int TreeListCtrl::LoadBase(UINT mask,const char* rfile)
{
	PrepFile(&m_TlU.m_treeRight.m_lfNode,rfile);
	m_TlU.m_treeLeft.m_lfNode.Curl();
	string lfile=m_TlU.m_treeLeft.m_lfNode.pListNode->GetPath();
	string lerr=m_TlU.m_treeLeft.m_lfNode.pErrNode->GetPath();
	string rerr=m_TlU.m_treeRight.m_lfNode.pErrNode->GetPath();
	int ret=Load(mask,lfile.c_str(),lerr.c_str(),rfile,rerr.c_str());
	if(ret==0)
		m_TlU.CacheNode();
	else
		m_TlU.m_strRecentPath.clear();
	return ret;
}
void TreeListCtrl::AllocCacheFile(ListFileNode** pFileNode)
{
	m_TlU.DestroyCacheNode();
	m_TlU.Fork();
	*pFileNode=&m_TlU.m_treeLeft.m_lfNode;
}
void TreeListCtrl::ResumeCacheFile()
{
	m_TlU.RestoreNode();
}
CString MingleListTitle(const string& left,const string& right)
{
	CString sep=_T("<->"),strMingle;
	if(left==""&&right=="")
		return CString();
	if(left==""||left==right)
		strMingle=a2t(right);
	else if(right=="")
		strMingle=a2t(left);
	if(strMingle==_T(""))
		strMingle.Format(_T("%s %s %s"),a2t(left),(LPCTSTR)sep,a2t(right));
	return strMingle;
}
static inline string get_file_title(const string& path)
{
	int pos=(int)path.rfind("\\");
	return pos==string::npos?path:path.substr(pos+1);
}
void TreeListCtrl::SetRealPath(int side,const string& path,uint notify_flag)
{
	if(side<0)
		m_TlU.m_treeLeft.m_strRealPath=process_path(path);
	else
		m_TlU.m_treeRight.m_strRealPath=process_path(path);
	UpdateListTitle(m_TlU.m_treeLeft.m_strRealPath,
		m_TlU.m_treeRight.m_strRealPath,notify_flag);
}
void UpdateListTitle(const string& left,const string& right,uint flags)
{
	if(flags&LIST_TITLE_UPDATE_PROP)
		SendMessageToIDWnd(IDW_PROP_WND,WM_SET_PROP_WND_TITLE,
			(WPARAM)&left,(LPARAM)&right);
	if(flags&LIST_TITLE_UPDATE_BASEBAR)
		SendMessageToIDWnd(IDW_BASE_BAR,WM_SET_PROP_WND_TITLE,
			(WPARAM)&left,(LPARAM)&right);
	if(flags&LIST_TITLE_UPDATE_CAPTION)
		PDXSetMainWndTitle(MingleListTitle(
			get_file_title(left),get_file_title(right)));
}
string TreeListCtrl::GetRealPath(int side)
{
	if(side<0)
		return m_TlU.m_treeLeft.m_strRealPath;
	else
		return m_TlU.m_treeRight.m_strRealPath;
}

