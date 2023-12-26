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
	while(CProgramData::GetBasePathNode()->PeekSub(idstr))
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
inline void PrepFile(ListFileNode* node,const char* file)
{
	assert(node->empty());
	node->pListNode=CProgramData::GetPathNode(file);
	node->pErrNode=CProgramData::GetErrListFileNode(node->pListNode);
	node->Curl();
}
int TreeListCtrl::LoadData(UINT mask,const char* lfile,const char* rfile)
{
	PrepFile(&m_TlU.m_treeLeft.m_lfNode,lfile);
	PrepFile(&m_TlU.m_treeRight.m_lfNode,rfile);
	return Load(mask,lfile,m_TlU.m_treeLeft.m_lfNode.pErrNode->GetPath().c_str(),
		rfile,m_TlU.m_treeRight.m_lfNode.pErrNode->GetPath().c_str());
}
int TreeListCtrl::LoadData(UINT mask,const char* rfile)
{
	PrepFile(&m_TlU.m_treeRight.m_lfNode,rfile);
	m_TlU.m_treeLeft.m_lfNode.Curl();
	int ret=Load(mask,m_TlU.m_treeLeft.m_lfNode.pListNode->GetPath().c_str(),
		m_TlU.m_treeLeft.m_lfNode.pErrNode->GetPath().c_str(),
		rfile,m_TlU.m_treeRight.m_lfNode.pErrNode->GetPath().c_str());
	if(ret==0)
		m_TlU.CacheNode();
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
void TreeListCtrl::DestroyCacheFile()
{
	m_TlU.DestroyCacheNode();
}