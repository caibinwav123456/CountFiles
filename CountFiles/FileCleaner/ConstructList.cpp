#include "pch.h"
#include "TreeListCtrl.h"
#include "utility.h"
#include <assert.h>
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
struct iterator_base
{
	typename vector<T>::iterator it;
	typename vector<T>::iterator end;
	FileListLoader* ctx;
	iterator_base(vector<T>& v,FileListLoader* loader):ctx(loader)
	{
		it=v.begin();
		end=v.end();
	}
	operator bool()
	{
		return it!=end;
	}
	void operator++(int)
	{
		if(it<end)
			it++;
	}
};
struct dir_iterator:public iterator_base<TLItemDir*>
{
	dir_iterator(vector<TLItemDir*>& dirlist,FileListLoader* loader):iterator_base<TLItemDir*>(dirlist,loader){}
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
struct errdir_iterator:public iterator_base<TLItemErrDir*>
{
	errdir_iterator(vector<TLItemErrDir*>& dirlist,FileListLoader* loader):iterator_base<TLItemErrDir*>(dirlist,loader){}
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
struct file_iterator:public iterator_base<TLItemFile*>
{
	file_iterator(vector<TLItemFile*>& flist,FileListLoader* loader):iterator_base<TLItemFile*>(flist,loader){}
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
struct errfile_iterator:public iterator_base<TLItemErrFile*>
{
	errfile_iterator(vector<TLItemErrFile*>& flist,FileListLoader* loader):iterator_base<TLItemErrFile*>(flist,loader){}
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
#define UNODE_ITERTYPE_DIR 0
#define UNODE_ITERTYPE_ERRDIR 1
#define UNODE_ITERTYPE_FILE 2
#define UNODE_ITERTYPE_ERRFILE 3
struct unode_iterator
{
	union
	{
		void* ptrit;
		dir_iterator* dirit;
		errdir_iterator* errdirit;
		file_iterator* fileit;
		errfile_iterator* errfileit;
	};
	uint type;
	bool master;
	unode_iterator(vector<TLItemDir*>* dirlist,FileListLoader* loader):type(UNODE_ITERTYPE_DIR),master(true)
	{
		dirit=new dir_iterator(*dirlist,loader);
	}
	unode_iterator(vector<TLItemFile*>* flist,FileListLoader* loader):type(UNODE_ITERTYPE_FILE),master(true)
	{
		fileit=new file_iterator(*flist,loader);
	}
	unode_iterator(vector<TLItemErrDir*>* dirlist,FileListLoader* loader):type(UNODE_ITERTYPE_ERRDIR),master(true)
	{
		errdirit=new errdir_iterator(*dirlist,loader);
	}
	unode_iterator(vector<TLItemErrFile*>* flist,FileListLoader* loader):type(UNODE_ITERTYPE_ERRFILE),master(true)
	{
		errfileit=new errfile_iterator(*flist,loader);
	}
	~unode_iterator()
	{
		if(!master)
			return;
		switch(type)
		{
		case UNODE_ITERTYPE_DIR:
			delete dirit;
			break;
		case UNODE_ITERTYPE_FILE:
			delete fileit;
			break;
		case UNODE_ITERTYPE_ERRDIR:
			delete errdirit;
			break;
		case UNODE_ITERTYPE_ERRFILE:
			delete errfileit;
			break;
		default:
			assert(false);
		}
	}
	unode_iterator(const unode_iterator& other):type(other.type),master(false)
	{
		ptrit=other.ptrit;
	}
	path_value_t operator*()
	{
		switch(type)
		{
		case UNODE_ITERTYPE_DIR:
			return **dirit;
			break;
		case UNODE_ITERTYPE_FILE:
			return **fileit;
			break;
		case UNODE_ITERTYPE_ERRDIR:
			return **errdirit;
			break;
		case UNODE_ITERTYPE_ERRFILE:
			return **errfileit;
			break;
		default:
			assert(false);
			return path_value_t();
		}
	}
	operator bool()
	{
		switch(type)
		{
		case UNODE_ITERTYPE_DIR:
			return *dirit;
			break;
		case UNODE_ITERTYPE_FILE:
			return *fileit;
			break;
		case UNODE_ITERTYPE_ERRDIR:
			return *errdirit;
			break;
		case UNODE_ITERTYPE_ERRFILE:
			return *errfileit;
			break;
		default:
			assert(false);
			return false;
		}
	}
	void operator++(int)
	{
		switch(type)
		{
		case UNODE_ITERTYPE_DIR:
			return (*dirit)++;
			break;
		case UNODE_ITERTYPE_FILE:
			return (*fileit)++;
			break;
		case UNODE_ITERTYPE_ERRDIR:
			return (*errdirit)++;
			break;
		case UNODE_ITERTYPE_ERRFILE:
			return (*errfileit)++;
			break;
		default:
			assert(false);
		}
	}
};
static int merge_callback(unode_iterator it1,unode_iterator it2,E_MERGE_SIDE side,void* param)
{
	assert((it1.type==UNODE_ITERTYPE_DIR&&it2.type==UNODE_ITERTYPE_ERRDIR)
		||(it1.type==UNODE_ITERTYPE_FILE&&it2.type==UNODE_ITERTYPE_ERRFILE));
	vector<TLItem*>* plist=(vector<TLItem*>*)param;
	TLItem *element1,*element2;
	switch(side)
	{
	case eMSLeft:
		element1=(it1.type==UNODE_ITERTYPE_DIR?(TLItem*)*(it1.dirit->it):(TLItem*)*(it1.fileit->it));
		element1->parentidx=plist->size();
		plist->push_back(element1);
		break;
	case eMSRight:
		element2=(it2.type==UNODE_ITERTYPE_DIR?(TLItem*)*(it2.errdirit->it):(TLItem*)*(it2.errfileit->it));
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
	if(subpairs!=NULL)
	{
		delete subpairs;
		subpairs=NULL;
	}
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
		unode_iterator itdir(&subdirs,ctx);
		unode_iterator iterrdir(&errdirs,ctx);
		merge_ordered_list(itdir,iterrdir,&merge_callback,(void*)&subitems);
		dir_border=subitems.size();
		unode_iterator itfile(&subfiles,ctx);
		unode_iterator iterrfile(&errfiles,ctx);
		merge_ordered_list(itfile,iterrfile,&merge_callback,(void*)&subitems);
		open_length=1+subitems.size();
	}
	catch(int err)
	{
		clear();
		return err;
	}
	return 0;
}
