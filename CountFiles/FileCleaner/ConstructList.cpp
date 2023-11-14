#include "pch.h"
#include "TreeListCtrl.h"
#include "utility.h"
#include <assert.h>
void TLItemSplice::clear()
{
	if(this==NULL)
		return;
	jntitems.clear();
#define restore_parentidx(pair,domain) if(pair.domain!=NULL)pair.domain->parentidx=-1;
	for(int i=0;i<(int)map.size();i++)
	{
		restore_parentidx(map[i],left);
		restore_parentidx(map[i],right);
	}
	map.clear();
	open_length=0;
	dir_border=0;
}
void TLItemDir::clear()
{
	clear_grp();
	TLItem** peer=GetPeerItem();
	if(peer!=NULL&&*peer!=NULL)
	{
		assert(dynamic_cast<TLItemDir*>(*peer)!=NULL);
		(dynamic_cast<TLItemDir*>(*peer))->clear_grp();
	}
}
void TLItemDir::clear_grp()
{
	subitems.clear();
	for(int i=0;i<(int)subfiles.size();i++)
	{
		subfiles[i]->Release();
		delete subfiles[i];
	}
	subfiles.clear();
	for(int i=0;i<(int)errfiles.size();i++)
	{
		errfiles[i]->Release();
		delete errfiles[i];
	}
	errfiles.clear();
	for(int i=0;i<(int)errdirs.size();i++)
	{
		errdirs[i]->Release();
		delete errdirs[i];
	}
	errdirs.clear();
	for(int i=0;i<(int)subdirs.size();i++)
	{
		subdirs[i]->Release();
		subdirs[i]->Detach();
		delete subdirs[i];
	}
	subdirs.clear();
	subpairs->clear();
	open_length=0;
	dir_border=0;
}
static inline void init_new_item(TLItem* item,TLItemDir* parent,E_TREE_ITEM_TYPE type,void* node,int idx)
{
	item->type=type;
	item->state=eFSEqual;
	item->node=node;
	item->parent=parent;
	item->org=idx;
	item->parentidx=-1;
}
static inline void new_dir_item(TLItemDir* parent,HDNODE node)
{
	TLItemDir* dir=new TLItemDir(parent->ctx);
	init_new_item(dir,parent,eITypeDir,node,parent->subdirs.size());
	parent->subdirs.push_back(dir);
}
static inline void new_file_item(TLItemDir* parent,HFNODE node)
{
	TLItemFile* file=new TLItemFile;
	init_new_item(file,parent,eITypeFile,node,parent->subfiles.size());
	parent->subfiles.push_back(file);
}
static inline void new_errdir_item(TLItemDir* parent,HENODE node)
{
	TLItemErrDir* dir=new TLItemErrDir;
	init_new_item(dir,parent,eITypeErrDir,node,parent->errdirs.size());
	dir->state=eFSError;
	parent->errdirs.push_back(dir);
}
static inline void new_errfile_item(TLItemDir* parent,HENODE node)
{
	TLItemErrFile* file=new TLItemErrFile;
	init_new_item(file,parent,eITypeErrFile,node,parent->errfiles.size());
	file->state=eFSError;
	parent->errfiles.push_back(file);
}
struct path_value_t
{
	string val;
	string* pval;
	path_value_t():pval(NULL){}
};
bool operator<(const path_value_t& a,const path_value_t& b)
{
	const string &stra=a.pval!=NULL?*a.pval:a.val,
		&strb=b.pval!=NULL?*b.pval:b.val;
	return compare_pathname(stra,strb)<0;
}
inline void get_node_name(TLItem* node,path_value_t& v,FileListLoader* ctx)
{
	file_node_info info;
	int ret=0;
	switch(node->type)
	{
	case eITypeDir:
		ret=ctx->GetNodeInfo(node->dirnode,&info);
		break;
	case eITypeFile:
		ret=ctx->GetNodeInfo(node->filenode,&info);
		break;
	default:
		assert(false);
	}
	if(ret!=0)
		throw ret;
	v.val=info.name;
}
inline void get_errnode_name(TLItem* node,path_value_t& v,FileListLoader* ctx)
{
	err_node_info info;
	int ret=ctx->GetNodeErrInfo(node->errnode,&info);
	if(ret!=0)
		throw ret;
	v.val=info.name;
}
template<class T>
struct iterator_base_ctx:public iterator_base<T>
{
	FileListLoader* ctx;
	iterator_base_ctx(vector<T>& v,FileListLoader* loader):iterator_base<T>(v),ctx(loader){}
};
struct dir_iterator:public iterator_base_ctx<TLItemDir*>
{
	dir_iterator(vector<TLItemDir*>& dirlist,FileListLoader* loader):iterator_base_ctx<TLItemDir*>(dirlist,loader){}
	path_value_t operator*()
	{
		path_value_t v;
		get_node_name(*it,v,ctx);
		return v;
	}
};
struct errdir_iterator:public iterator_base_ctx<TLItemErrDir*>
{
	errdir_iterator(vector<TLItemErrDir*>& dirlist,FileListLoader* loader):iterator_base_ctx<TLItemErrDir*>(dirlist,loader){}
	path_value_t operator*()
	{
		path_value_t v;
		get_errnode_name(*it,v,ctx);
		return v;
	}
};
struct file_iterator:public iterator_base_ctx<TLItemFile*>
{
	file_iterator(vector<TLItemFile*>& flist,FileListLoader* loader):iterator_base_ctx<TLItemFile*>(flist,loader){}
	path_value_t operator*()
	{
		path_value_t v;
		get_node_name(*it,v,ctx);
		return v;
	}
};
struct errfile_iterator:public iterator_base_ctx<TLItemErrFile*>
{
	errfile_iterator(vector<TLItemErrFile*>& flist,FileListLoader* loader):iterator_base_ctx<TLItemErrFile*>(flist,loader){}
	path_value_t operator*()
	{
		path_value_t v;
		get_errnode_name(*it,v,ctx);
		return v;
	}
};
struct grp_dir_iterator:public iterator_base_ctx<TLItem*>
{
	grp_dir_iterator(vector<TLItem*>& itemlist,uint border,FileListLoader* loader):iterator_base_ctx<TLItem*>(itemlist,loader)
	{
		end=itemlist.begin()+border;
	}
	path_value_t operator*()
	{
		path_value_t v;
		switch((*it)->type)
		{
		case eITypeDir:
			get_node_name(*it,v,ctx);
			break;
		case eITypeErrDir:
			get_errnode_name(*it,v,ctx);
			break;
		default:
			assert(false);
		}
		return v;
	}
};
struct grp_file_iterator:public iterator_base_ctx<TLItem*>
{
	grp_file_iterator(vector<TLItem*>& itemlist,uint border,FileListLoader* loader):iterator_base_ctx<TLItem*>(itemlist,loader)
	{
		it=itemlist.begin()+border;
	}
	path_value_t operator*()
	{
		path_value_t v;
		switch((*it)->type)
		{
		case eITypeFile:
			get_node_name(*it,v,ctx);
			break;
		case eITypeErrFile:
			get_errnode_name(*it,v,ctx);
			break;
		default:
			assert(false);
		}
		return v;
	}
};
static int merge_callback_dir(dir_iterator it1,errdir_iterator it2,E_MERGE_SIDE side,void* param)
{
	vector<TLItem*>* plist=(vector<TLItem*>*)param;
	TLItem *element1,*element2;
	switch(side)
	{
	case eMSLeft:
		element1=*(it1.it);
		plist->push_back(element1);
		break;
	case eMSRight:
		element2=*(it2.it);
		plist->push_back(element2);
		break;
	default:
		assert(false);
	}
	return 0;
}
static int merge_callback_file(file_iterator it1,errfile_iterator it2,E_MERGE_SIDE side,void* param)
{
	vector<TLItem*>* plist=(vector<TLItem*>*)param;
	TLItem *element1,*element2;
	switch(side)
	{
	case eMSLeft:
		element1=*(it1.it);
		plist->push_back(element1);
		break;
	case eMSRight:
		element2=*(it2.it);
		plist->push_back(element2);
		break;
	default:
		assert(false);
	}
	return 0;
}
static int merge_callback_grp_dir(grp_dir_iterator it1,grp_dir_iterator it2,E_MERGE_SIDE side,void* param)
{
	assert((*it1.it)->type==eITypeDir||(*it1.it)->type==eITypeErrDir);
	assert((*it2.it)->type==eITypeDir||(*it2.it)->type==eITypeErrDir);
	TLItemSplice* splice=(TLItemSplice*)param;
	TLItemPair tuple;
	int ret=0;
	switch(side)
	{
	case eMSLeft:
		tuple.left=*it1.it;
		if(tuple.left->type==eITypeDir)
			tuple.left->state=eFSSolo;
		tuple.right=NULL;
		break;
	case eMSRight:
		tuple.left=NULL;
		tuple.right=*it2.it;
		if(tuple.right->type==eITypeDir)
			tuple.right->state=eFSSolo;
		break;
	case eMSBoth:
		tuple.left=*it1.it;
		tuple.right=*it2.it;
		if(tuple.left->type==eITypeDir&&tuple.right->type==eITypeDir)
		{
			file_node_info info1,info2;
			fail_op(ret,0,it1.ctx->GetNodeInfo(tuple.left->dirnode,&info1),throw ret);
			fail_op(ret,0,it2.ctx->GetNodeInfo(tuple.right->dirnode,&info2),throw ret);
			if(info1.size==UInteger64(0)&&info2.size!=UInteger64(0))
				tuple.right->state=eFSSolo;
			else if(info1.size!=UInteger64(0)&&info2.size==UInteger64(0))
				tuple.left->state=eFSSolo;
			else if(info1.mod_time<info2.mod_time)
			{
				tuple.left->state=eFSOld;
				tuple.right->state=eFSNew;
			}
			else if(info1.mod_time>info2.mod_time)
			{
				tuple.left->state=eFSNew;
				tuple.right->state=eFSOld;
			}

			TLItemDir *ldir=(TLItemDir*)tuple.left,
				*rdir=(TLItemDir*)tuple.right;
			ldir->subpairs=rdir->subpairs=new TLItemSplice;
		}
		break;
	}
	splice->map.push_back(tuple);
	splice->jntitems.push_back(&splice->map.back());
	return 0;
}
static int merge_callback_grp_file(grp_file_iterator it1,grp_file_iterator it2,E_MERGE_SIDE side,void* param)
{
	assert((*it1.it)->type==eITypeFile||(*it1.it)->type==eITypeErrFile);
	assert((*it2.it)->type==eITypeFile||(*it2.it)->type==eITypeErrFile);
	TLItemSplice* splice=(TLItemSplice*)param;
	TLItemPair tuple;
	int ret=0;
	switch(side)
	{
	case eMSLeft:
		tuple.left=*it1.it;
		if(tuple.left->type==eITypeFile)
			tuple.left->state=eFSSolo;
		tuple.right=NULL;
		break;
	case eMSRight:
		tuple.left=NULL;
		tuple.right=*it2.it;
		if(tuple.right->type==eITypeFile)
			tuple.right->state=eFSSolo;
		break;
	case eMSBoth:
		tuple.left=*it1.it;
		tuple.right=*it2.it;
		if(tuple.left->type==eITypeFile&&tuple.right->type==eITypeFile)
		{
			file_node_info info1,info2;
			fail_op(ret,0,it1.ctx->GetNodeInfo(tuple.left->filenode,&info1),throw ret);
			fail_op(ret,0,it2.ctx->GetNodeInfo(tuple.right->filenode,&info2),throw ret);
			if(info1.mod_time<info2.mod_time)
			{
				tuple.left->state=eFSOld;
				tuple.right->state=eFSNew;
			}
			else if(info1.mod_time>info2.mod_time)
			{
				tuple.left->state=eFSNew;
				tuple.right->state=eFSOld;
			}
		}
		break;
	}
	splice->map.push_back(tuple);
	splice->jntitems.push_back(&splice->map.back());
	return 0;
}
int join_list(TLItemDir* llist,TLItemDir* rlist)
{
	assert(llist->subpairs!=NULL&&rlist->subpairs!=NULL);
	assert(llist->subpairs==rlist->subpairs);
	TLItemSplice* splice=llist->subpairs;
	try
	{
		grp_dir_iterator grpitdir1(llist->subitems,llist->dir_border,&llist->ctx->m_ListLoader);
		grp_dir_iterator grpitdir2(rlist->subitems,rlist->dir_border,&rlist->ctx->m_ListLoader);
		merge_ordered_list(grpitdir1,grpitdir2,merge_callback_grp_dir,(void*)splice);
		splice->dir_border=splice->map.size();
		grp_file_iterator grpitfile1(llist->subitems,llist->dir_border,&llist->ctx->m_ListLoader);
		grp_file_iterator grpitfile2(rlist->subitems,rlist->dir_border,&rlist->ctx->m_ListLoader);
		merge_ordered_list(grpitfile1,grpitfile2,merge_callback_grp_file,(void*)splice);
		splice->open_length=1+splice->map.size();
	}
	catch(int err)
	{
		llist->clear();
		rlist->clear();
		return err;
	}
	return 0;
}
int TLItemDir::construct_list()
{
	int ret=0;
	fail_goto(ret,0,construct_list_grp(),fail);
	TLItem** peer=GetPeerItem();
	if(peer!=NULL&&*peer!=NULL)
	{
		TLItemPair* tuple=GetCouple();
		TLItemDir* dir=dynamic_cast<TLItemDir*>(*peer);
		assert(dir!=NULL);
		fail_goto(ret,0,dir->construct_list_grp(),fail);
		fail_goto(ret,0,join_list((TLItemDir*)tuple->left,(TLItemDir*)tuple->right),fail);
		for(int i=0;i<(int)subpairs->jntitems.size();i++)
		{
			TLItemPair* couple=subpairs->jntitems[i];
			assert_valid_tuple(couple);
			if(couple->left!=NULL)
				couple->left->parentidx=i;
			if(couple->right!=NULL)
				couple->right->parentidx=i;
			if (couple->left != NULL)
				couple->left->update_state();
			else
				couple->right->update_state();
		}
	}
	else
	{
		for(int i=0;i<(int)subitems.size();i++)
		{
			subitems[i]->parentidx=i;
			subitems[i]->update_state();
		}
	}
	return 0;
fail:
	clear();
	return ret;
}
int TLItemDir::construct_list_grp()
{
	int ret=0;
	bool construct_all=open_length==0;
	if(construct_all)
	{
		assert(open_length==0&&dir_border==0);
		assert(subitems.empty());
		assert(subdirs.empty());
		assert(subfiles.empty());
		assert(errdirs.empty());
		assert(errfiles.empty());
		assert(subpairs==NULL);
	}
	subitems.clear();
	subpairs->clear();
	if(construct_all)
	{
		int cnt=0;
		cnt=get_subdir_cnt(dirnode);
		for(int i=0;i<cnt;i++)
		{
			new_dir_item(this,get_subdir(dirnode,i));
		}
		cnt=get_subfile_cnt(dirnode);
		for(int i=0;i<cnt;i++)
		{
			new_file_item(this,get_subfile(dirnode,i));
		}
		cnt=get_errdir_cnt(dirnode);
		for(int i=0;i<cnt;i++)
		{
			new_errdir_item(this,get_errdir(dirnode,i));
		}
		cnt=get_errfile_cnt(dirnode);
		for(int i=0;i<cnt;i++)
		{
			new_errfile_item(this,get_errfile(dirnode,i));
		}
	}
	else
	{
		for(int i=0;i<(int)subdirs.size();i++)
		{
			subdirs[i]->parentidx=-1;
		}
		for(int i=0;i<(int)subfiles.size();i++)
		{
			subfiles[i]->parentidx=-1;
		}
		for(int i=0;i<(int)errdirs.size();i++)
		{
			errdirs[i]->parentidx=-1;
		}
		for(int i=0;i<(int)errfiles.size();i++)
		{
			errfiles[i]->parentidx=-1;
		}
	}
	try
	{
		dir_iterator itdir(subdirs,&ctx->m_ListLoader);
		errdir_iterator iterrdir(errdirs,&ctx->m_ListLoader);
		merge_ordered_list(itdir,iterrdir,&merge_callback_dir,(void*)&subitems);
		dir_border=subitems.size();
		file_iterator itfile(subfiles,&ctx->m_ListLoader);
		errfile_iterator iterrfile(errfiles,&ctx->m_ListLoader);
		merge_ordered_list(itfile,iterrfile,&merge_callback_file,(void*)&subitems);
		open_length=1+subitems.size();
	}
	catch(int err)
	{
		clear();
		return err;
	}
	return 0;
}
