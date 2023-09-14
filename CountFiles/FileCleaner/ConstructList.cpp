#include "pch.h"
#include "TreeListCtrl.h"
#include "utility.h"
#include <assert.h>
void TLItemDir::clear()
{
	subitems.clear();
	subpairs->clear();
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
		subdirs[i]->Detach();
		subdirs[i]->Release();
		delete subdirs[i];
	}
	subdirs.clear();
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
		file_node_info info;
		int ret=ctx->GetNodeInfo((*it)->dirnode,&info);
		if(ret!=0)
			throw ret;
		v.val=info.name;
		return v;
	}
};
struct errdir_iterator:public iterator_base_ctx<TLItemErrDir*>
{
	errdir_iterator(vector<TLItemErrDir*>& dirlist,FileListLoader* loader):iterator_base_ctx<TLItemErrDir*>(dirlist,loader){}
	path_value_t operator*()
	{
		path_value_t v;
		err_node_info info;
		int ret=ctx->GetNodeErrInfo((*it)->errnode,&info);
		if(ret!=0)
			throw ret;
		v.val=info.name;
		return v;
	}
};
struct file_iterator:public iterator_base_ctx<TLItemFile*>
{
	file_iterator(vector<TLItemFile*>& flist,FileListLoader* loader):iterator_base_ctx<TLItemFile*>(flist,loader){}
	path_value_t operator*()
	{
		path_value_t v;
		file_node_info info;
		int ret=ctx->GetNodeInfo((*it)->filenode,&info);
		if(ret!=0)
			throw ret;
		v.val=info.name;
		return v;
	}
};
struct errfile_iterator:public iterator_base_ctx<TLItemErrFile*>
{
	errfile_iterator(vector<TLItemErrFile*>& flist,FileListLoader* loader):iterator_base_ctx<TLItemErrFile*>(flist,loader){}
	path_value_t operator*()
	{
		path_value_t v;
		err_node_info info;
		int ret=ctx->GetNodeErrInfo((*it)->errnode,&info);
		if(ret!=0)
			throw ret;
		v.val=info.name;
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
		element1->parentidx=plist->size();
		plist->push_back(element1);
		break;
	case eMSRight:
		element2=*(it2.it);
		element2->parentidx=plist->size();
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
		element1->parentidx=plist->size();
		plist->push_back(element1);
		break;
	case eMSRight:
		element2=*(it2.it);
		element2->parentidx=plist->size();
		plist->push_back(element2);
		break;
	default:
		assert(false);
	}
	return 0;
}
int TLItemDir::construct_list()
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
		dir_iterator itdir(subdirs,ctx);
		errdir_iterator iterrdir(errdirs,ctx);
		merge_ordered_list(itdir,iterrdir,&merge_callback_dir,(void*)&subitems);
		dir_border=subitems.size();
		file_iterator itfile(subfiles,ctx);
		errfile_iterator iterrfile(errfiles,ctx);
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
